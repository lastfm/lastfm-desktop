/*
   Copyright 2009 Last.fm Ltd. 
   Copyright 2009 John Stamp <jstamp@users.sourceforge.net>

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QFile>
#include <QStringList>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>

#include <lastfm/Fingerprint.h>
#include <lastfm/FingerprintableSource.h>
#include <lastfm/Track.h>

#include "MadSource.h"
#include "VorbisSource.h"
#include "FlacSource.h"
#include "AacSource.h"

#include "Fingerprinter.h"


int typeOf(const QString& path);
lastfm::FingerprintableSource* factory(int type);
enum { MP3, OGG, FLAC, AAC, UNKNOWN };

Fingerprinter::Fingerprinter( const lastfm::Track& track, QObject* parent )
    :QThread( parent ), m_track( track )
{
}

void
Fingerprinter::run()
{
    QEventLoop loop;

    lastfm::Fingerprint fingerprint( m_track );

    if ( fingerprint.id().isNull() )
    {
        lastfm::FingerprintableSource* fingerprintableSource = factory( typeOf( m_track.url().toLocalFile() ) );

        if ( fingerprintableSource )
        {
            try
            {
                fingerprint.generate( fingerprintableSource );

                QNetworkReply* fingerprintReply = fingerprint.submit();
                connect( fingerprintReply, SIGNAL(finished()), &loop, SLOT(quit()) );
                loop.exec();

                fingerprint.decode( fingerprintReply );

                qDebug() << "Fingerprint success: " << fingerprint.id();

#ifndef NDEBUG
                // This code will fetch the suggestions from the fingerprint id, one
                // day we might do something with this info, like offer corrections

                QNetworkReply* reply = fingerprint.id().getSuggestions();
                connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
                loop.exec();

                QMap<float,Track> suggestions = lastfm::FingerprintId::getSuggestions( reply );

                qDebug() << suggestions;
#endif
            }
            catch ( const lastfm::Fingerprint::Error& error )
            {
                qWarning() << "Fingerprint error: " << error;
            }
            delete fingerprintableSource;
        }
    }
}

lastfm::FingerprintableSource* factory(int type)
{
    switch (type) {
        case MP3: return new MadSource;
        case OGG: return new VorbisSource;
        case FLAC: return new FlacSource;
        case AAC: return new AacSource;
        default: return 0;
    }
}

int typeOf(const QString& fileName)
{
    QStringList parts = fileName.split( "." );
    QString extension;
    if ( parts.size() > 1 )
        extension = parts.last();

    // Let's be trusting about extensions
    if ( extension.toLower() == "mp3" )
        return MP3;
    else if ( extension.toLower() == "ogg" )
        return OGG;
    else if ( extension.toLower() == "oga" )
        return FLAC;
    else if ( extension.toLower() == "flac" )
        return FLAC;
    else if ( extension.toLower() == "aac" )
        return AAC;
    else if ( extension.toLower() == "m4a" )
        return AAC;

    // So much for relying on extensions.  Let's try file magic instead.
    FILE *fp = NULL;
    unsigned char header[35];

    fp = fopen(QFile::encodeName(fileName), "rb");
    if ( !fp )
    {
        return UNKNOWN;
    }
    int fType = UNKNOWN;
    fread( header, 1, 35, fp );

    // Some formats can have ID3 tags (or not), so let's just
    // get them out of the way first before we check what we have.
    if ( memcmp( header, "ID3", 3) == 0 )
    {
        int tagsize = 0;
        /* high bit is not used */
        tagsize = (header[6] << 21) | (header[7] << 14) |
            (header[8] <<  7) | (header[9] <<  0);

        tagsize += 10;
        fseek( fp, tagsize, SEEK_SET );
        fread( header, 1, 35, fp );
    }

    if ( (header[0] == 0xFF) && ((header[1] & 0xFE) == 0xFA ) )
    {
        fType = MP3;
    }
    else if ( memcmp(header, "OggS", 4) == 0 )
    {
        if ( memcmp(&header[29], "vorbis", 6) == 0 )
        {
            // ogg vorbis (.ogg)
            fType = OGG;
        }
        else if ( memcmp(&header[29], "FLAC", 4) == 0 )
        {
            // ogg flac (.oga)
            fType = FLAC;
        }
    }
    else if ( memcmp(header, "fLaC", 4 ) == 0 )
    {
        // flac file
        fType = FLAC;
    }
    else if ( (header[0] == 0xFF) && ((header[1] & 0xF6) == 0xF0) )
    {
        // aac adts
        fType = AAC;
    }
    else if (memcmp(header, "ADIF", 4) == 0)
    {
        // aac adif
        fType = AAC;
    }
    else if ( memcmp( &header[4], "ftyp", 4 ) == 0 )
    {
        // mp4 header: aac
        fType = AAC;
    }

    fclose(fp);
    return fType;
}
