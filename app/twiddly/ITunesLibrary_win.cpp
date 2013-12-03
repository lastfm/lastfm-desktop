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
#include "ITunesLibrary.h"
#include "IPodScrobble.h"
#include "common/qt/msleep.cpp"
#include "plugins/iTunes/ITunesComWrapper.h"
#include <cassert>
#include <QDateTime>
#include <QFileInfo>
#include <QDebug>

using namespace std;


ITunesLibrary::ITunesLibrary( const QString&, bool const iPod ) :
    m_currentIndex( 0 ),
    m_isIPod( iPod ),
    m_trackCount( -1 )
{
    m_com = new ITunesComWrapper();

    // This will throw if it fails
    if ( !iPod )
        m_trackCount = m_com->libraryTrackCount();
    else
    {
        // loop 20 twenty times as sometimes the iPod isn't in iTunes yet and
        // the plugin has called us prematurely
        for( int x = 0; x < 20 && m_trackCount == -1; x++)
        {
            if (x) Qt::msleep( 500 );
            // the first ipod is used, which isn't perfect, but oh well
            m_trackCount = m_com->iPodLibraryTrackCount();
        }
    }

    qDebug() << "Found " << m_trackCount << " tracks in iTunes library";

}


ITunesLibrary::~ITunesLibrary()
{
    delete m_com;
}
    

bool
ITunesLibrary::hasTracks() const
{
    return (int)m_currentIndex < m_trackCount;
}


int
ITunesLibrary::trackCount() const
{
    return m_trackCount;
}


ITunesLibrary::Track
ITunesLibrary::nextTrack()
{
    // Weird increment is because if this call throws we still want to
    // have increased m_currentIndex. Otherwise we can get stuck in an
    // infinite loop calling this function over and over.
    COM::ITunesTrack it = m_com->track( (++m_currentIndex) - 1 );

    ITunesLibrary::Track t;
    t.d = (m_isIPod)
            ? new IPodLibraryTrackData( it )
            : new ITunesLibraryTrackData( it );
    return t;
}


Track
ITunesLibrary::Track::lastfmTrack() const
{
    // TODO: why are we doing this? It will return true for manual tracks as it checks the path.
    if ( isNull() )
        return Track();

    COM::ITunesTrack i = d->i;

    // These will throw if something goes wrong
    IPodScrobble t;
    t.setArtist( QString::fromStdWString( i.artist() ) );
    t.setAlbumArtist( QString::fromStdWString( i.albumArtist() ) );
    t.setTitle( QString::fromStdWString( i.track() ) );
    t.setDuration( i.duration() );
    t.setAlbum( QString::fromStdWString( i.album() ) );
    t.setPlayCount( i.playCount() );
    t.setPodcast( i.podcast() );
    t.setVideo( i.video() );

    QDateTime stamp = QDateTime::fromString( QString::fromStdWString( i.lastPlayed() ), "yyyy-MM-dd hh:mm:ss" );
    if ( stamp.isValid() )
    {
        uint unixTime = stamp.toTime_t();

        // This is to prevent the spurious scrobble bug that can happen if iTunes is
        // shut during twiddling. The timestamp returned in that case was FFFFFFFF
        // so let's check for that.
        if ( unixTime == 0xFFFFFFFF )
        {
			qWarning() << "Caught a 0xFFFFFFFF timestamp, assume it's the scrobble of spury:" << QString::fromStdWString( i.toString() );
            return Track();
        } 
        
        t.setTimeStamp( stamp );
    }
    else
    {
        // If we don't have a valid timestamp, set to current time. Should work. We hope.
        qWarning() << "Invalid timestamp, set to 60 seconds ago:" << QString::fromStdWString( i.toString() );
        t.setTimeStamp( QDateTime::currentDateTime().addSecs( -60 ) );
    }

    const QString path = QString::fromStdWString( i.path() );

	t.setUrl( QUrl::fromLocalFile( QFileInfo( path ).absoluteFilePath() ) );
    t.setSource( Track::MediaDevice );

    return t;
}
