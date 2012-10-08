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

#ifdef FFMPEG_FINGERPRINTING
#include "LAV_Source.h"
#endif

#include "Fingerprinter.h"


Fingerprinter::Fingerprinter( const lastfm::Track& track, QObject* parent )
    :QThread( parent ), m_track( track )
{
}

void
Fingerprinter::run()
{
#ifdef FFMPEG_FINGERPRINTING
    QEventLoop loop;

    lastfm::Fingerprint fingerprint( m_track );

    if ( fingerprint.id().isNull() )
    {
        lastfm::FingerprintableSource* fingerprintableSource = new LAV_Source();

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
#endif // FFMPEG_FINGERPRINTING
}
