/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole, Erik Jaelevik, 
        Christian Muehlhaeuser

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "lib/unicorn/mac/AppleScript.h"
#include "IPod.h"
#include "IPodScrobble.h"
#include "ITunesLibrary.h"
#include "PlayCountsDatabase.h"
#include "common/qt/msleep.cpp"
#include "plugins/iTunes/ITunesExceptions.h"
#include <lastfm/misc.h>
#include <lastfm/Track.h>
#include <QtCore>
#include <QtXml>
#include <iostream>


IPod* //static
IPod::fromCommandLineArguments( const QStringList& args )
{
#if 0
    // handy for debug
    QStringList myArgs = QString( "--device ipod --connection usb --pid 4611 --vid 1452 --serial 000000C8B035" ).split( ' ' );
    //QStringList myArgs = QString( "--bootstrap" ).split( ' ' );
    #define args myArgs
#endif



    QMap<QString, QString> map;
    QListIterator<QString> i( args );
    while (i.hasNext())
    {
        QString arg = i.next();
        if (!arg.startsWith( "--" ) || !i.hasNext() || i.peekNext().startsWith( "--" ))
            continue;
        arg = arg.mid( 2 );

        // get all the strings up until the end or the next --
        QString value = i.next();
        while ( i.hasNext() && !i.peekNext().startsWith("--") )
            value.append( " " + i.next() );

        map[arg] = value;
    }

    IPod* ipod;
    if (args.contains( "--manual" ))
        ipod = new ManualIPod;
    else
        ipod = new AutomaticIPod;

    #define THROW_IF_EMPTY( x ) ipod->x = map[#x]; if (ipod->x.isEmpty()) throw "Could not resolve argument: --" #x;

    THROW_IF_EMPTY( device );
    THROW_IF_EMPTY( vid );
    THROW_IF_EMPTY( pid );
    THROW_IF_EMPTY( serial );

    // use device for name if name wasn't sent
    ipod->name = map["name"];
    if (ipod->name.isEmpty())
        ipod->name = ipod->serial;
    
    #undef THROW_IF_EMPTY
    
    return ipod;
}


QString
IPod::scrobbleId() const
{
#ifdef Q_OS_MAC
    const char* os = "Macintosh";
#elif defined Q_OS_WIN
    const char* os = "Windows";
#else
    #error What kind of operating system are you?! \
           I am a home-made one.
#endif

    return vid + '-' + pid + '-' + device + '-' + os;
}


QDir
IPod::saveDir() const 
{
    QDir d = lastfm::dir::runtimeData().filePath( "devices/" + uid() );
    d.mkpath( "." );
    return d;
}


QDomDocument
IPod::ScrobbleList::xml() const
{    
    QDomDocument xml;
    QDomElement root = xml.createElement( "submissions" );
    root.setAttribute( "product", "Twiddly" );

    QListIterator<Track> i( *this );
    while (i.hasNext())
        root.appendChild( i.next().toDomElement( xml ) );
    
    xml.appendChild( root );
    
    return xml;
}


/** iPod plays are determined using our iTunes Plays sqlite database and
  * comparing that with the actual state of the iTunes Library database after
  * an iPod is synced with it
  */
void
IPod::twiddle()
{
    PlayCountsDatabase& db = *playCountsDatabase();
    ITunesLibrary& library = *iTunesLibrary();

    QList<ITunesLibrary::Track> tracksToUpdate;
    QList<ITunesLibrary::Track> tracksToInsert;
    QList<ITunesLibrary::Track> tracksToScrobble;

    int nullTrackCount = 0;

    // If creation of the library class failed due to a dialog showing in iTunes
    // or COM not responding for some other reason, hasTracks will just return false
    // and we will quit this run.
    while ( library.hasTracks() )
    {
        try
        {
            ITunesLibrary::Track track = library.nextTrack();

            if ( track.isNull() )
            {
                // Failed to read current iTunes track. Either something went wrong,
                // or the track was not found on the disk despite being in the iTunes library.

                // Don't log every error as this could be a library full of iTunes Match tracks
                // just count how many failed and say at the end

                ++nullTrackCount;
                continue;
            }

            QString id = track.uniqueId();

            // We don't know about this track yet, this means either:-
            //   1. The track was added to iTunes since the last sync. thus it is
            //      impossible for it to have been played on the iPod
            //   2. On Windows, the path of the track changed since the last sync.
            //      Since we don't have persistent IDs on Windows we have no way of 
            //      matching up this track up with its previous incarnation. Thus
            //      we don't scrobble it as we have no idea if it was played or not
            //      chances are, it wasn't
            PlayCountsDatabase::Track dbTrack = db[id];

            if ( dbTrack.isNull() )
            {
                tracksToInsert << track;
                continue;
            }

            const int diff = track.playCount() - dbTrack.playCount(); // can throw
        
            if ( diff > 0 )
                tracksToScrobble << track;

            // a worthwhile optimisation since updatePlayCount() is really slow
            // NOTE negative diffs *are* possible
            if ( diff < 0 )
                tracksToUpdate << track; // can throw
        }
        catch ( ITunesException& )
        {
            // Carry on...
        }
    }

    qDebug() << "There were " << nullTrackCount << " null tracks";

    if ( tracksToUpdate.count() + tracksToInsert.count() + tracksToScrobble.count() > 0 )
    {
        // We've got some updates and inserts to do so lock the database and do them

        db.beginTransaction();

        foreach ( const ITunesLibrary::Track& track, tracksToScrobble )
        {
            try
            {
                ::Track t = track.lastfmTrack(); // can throw

                // Because we take a snapshot of our playcounts db there is a possible race condition.
                // use db.track here to fetch the latest playcount now that we've locked the database
                const int diff = track.playCount() - db.track( track.uniqueId() ).playCount();

                if ( !t.isNull() )
                {
                    if ( t.timestamp().secsTo( QDateTime::currentDateTime() ) > 30 )
                    {
                        // we only scrobble tracks with a timestamp older than 30 seconds
                        // to give the iTunes plugin time so update the playcount db
                        // after a track change - bit of a hack, but it stops spurious iPod scrobbles

                        // update the playcount db to the current playcount for this track
                        // this means that we won't try to scrobble the track again
                        db.update( track );

                        IPodScrobble t2( t );
                        t2.setPlayCount( diff );
                        t2.setMediaDeviceId( scrobbleId() );
                        t2.setUniqueId( track.uniqueId() );
                        m_scrobbles += t2;
                        qDebug() << diff << "scrobbles found for" << t;
                    }
                    else
                    {
                        qDebug() << "Timestamp less than 30 seconds. Don't scrobble yet.";
                    }
                }
                else
                {
                    qWarning() << "Couldn't get Track for" << track.uniqueId();

                    // We get here if COM fails to populate the Track for whatever reason.
                    // Therefore we continue and don't let the local db update. That way we
                    // maintain the diff and we should be picking up on it next time twiddly
                    // runs.
                    continue;
                }
            }
            catch ( ITunesException& )
            {
                // Carry on...
            }
        }

        // this should just be tracks with negative playcount diffs
        foreach ( const ITunesLibrary::Track& track, tracksToUpdate )
            db.update( track );

        // insert all the new tracks we've found
        foreach ( const ITunesLibrary::Track& track, tracksToInsert )
            db.insert( track );

        db.endTransaction();
    }

    delete &db;
    delete &library;
}


#ifdef Q_OS_MAC
    ManualIPod::ManualIPod()
               : m_pid( firstPid() )
    {}


    QString //static
    ManualIPod::firstPid()
    {
        AppleScript script;
        script << "tell application 'iTunes' to "
                  "return persistent ID of some source whose kind is iPod";
                  
        // wait for iPod to exist, since we're launched early by the plugin
        // and it may not be in iTunes yet, 10 seconds is more than enough
        // without being excessive
        QString pid;
        for( int x = 0; x < 20 && (pid = script.exec()).isEmpty(); ++x )
        {
            Qt::msleep( 500 );
        }
                
        return pid;
    }


#else
    ManualIPod::ManualIPod()
    {}
#endif
