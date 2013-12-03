/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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

#include <lastfm/misc.h>

#include "ITunesListener.h"
#include "../PlayerConnection.h"

struct ITunesConnection : PlayerConnection
{
    ITunesConnection() : PlayerConnection( "osx", "iTunes" )
    {}
    
    void start( const Track& t )
    {
        MutableTrack mt( t );
        mt.setSource( Track::Player );
        mt.setExtra( "playerId", id() );
        mt.setExtra( "playerName", name() );
        mt.stamp();
        handleCommand( CommandStart, t ); 
    }

    void pause() { handleCommand( CommandPause ); }
    void resume() { handleCommand( CommandResume ); }
    void stop() { handleCommand( CommandStop ); }
};


ITunesListener::ITunesListener( QObject* parent )
    :QObject( parent ), m_connection( 0 )
{
    qRegisterMetaType<Track>("Track");

    m_currentTrackScript = AppleScript("tell application \"iTunes\" to tell current track\n"
                                          "try\n"
                                              "set L to location\n"
                                              "set L to POSIX path of L\n"
                                          "on error\n"
                                              "set L to \"\"\n"
                                          "end try\n"
                                          "return artist & \"\n\" & album artist & \"\n\" & album & \"\n\" & name & \"\n\" & (duration as integer) & \"\n\" & L & \"\n\" & persistent ID & \"\n\" & podcast & \"\n\" & video kind\n"
                                      "end tell\n" );

    CFNotificationCenterAddObserver( CFNotificationCenterGetDistributedCenter(),
                                    this,
                                    callback,
                                    CFSTR( "com.apple.iTunes.playerInfo" ),
                                    NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately );

    QMetaObject::invokeMethod( this, "setupCurrentTrack", Qt::QueuedConnection );
}


ITunesListener::~ITunesListener()
{
    delete m_connection;
}

    
void
ITunesListener::callback( CFNotificationCenterRef, void* observer, CFStringRef, const void*, CFDictionaryRef info )
{    
    static_cast<ITunesListener*>(observer)->callback( info );
}


/*******************************************************************************
  * code-clarity-class used by callback() */
class ITunesDictionaryHelper
{   
    CFDictionaryRef const m_info; // leave here or risk an initialisation bug
    
public:
    ITunesDictionaryHelper( CFDictionaryRef info ) 
            : m_info( info ), 
              state( getState() )
    {}

    void determineTrackInformation();
        
    QString artist, name, album, path, pid;
    int duration;
    ITunesListener::State const state; // *MUST* be after m_info
    
private:
    template <typename T> T
    token( CFStringRef t )
    {
        return (T) CFDictionaryGetValue( m_info, t );
    }

    ITunesListener::State
    getState()
    {
        CFStringRef state = token<CFStringRef>( CFSTR("Player State") );

        #define compare( x ) if (CFStringCompare( state, CFSTR( #x ), 0 ) == kCFCompareEqualTo) return ITunesListener::x
        compare( Playing );
        compare( Paused );
        compare( Stopped );
        #undef compare
        
        return ITunesListener::Unknown;
    }
};


template <> QString
ITunesDictionaryHelper::token<QString>( CFStringRef t )
{
    CFStringRef s = token<CFStringRef>( t );
    return lastfm::CFStringToQString( s );
}   


template <> int
ITunesDictionaryHelper::token<int>( CFStringRef t )
{
    int i = 0;
    CFNumberRef n = token<CFNumberRef>( t );
    if (n) CFNumberGetValue( n, kCFNumberIntType, &i );
    return i;
}


void
ITunesDictionaryHelper::determineTrackInformation()
{
    duration = token<int>( CFSTR("Total Time") ) / 1000;
    artist = token<QString>( CFSTR("Artist") );
    album = token<QString>( CFSTR("Album") );
    name = token<QString>( CFSTR("Name") );
    pid = QString::number( token<int>( CFSTR("PersistentID") ) );
    
    // Get path decoded - iTunes encodes the file location as URL
    CFStringRef location = token<CFStringRef>( CFSTR("Location") );
    QUrl url = QUrl::fromEncoded( lastfm::CFStringToUtf8( location ) );
    path = url.toString().remove( "file://localhost" );
}
/******************************************************************************/


static inline QString
encodeAmp( QString data ) 
{ 
    return data.replace( '&', "&&" );
}


void
ITunesListener::callback( CFDictionaryRef info )
{
    if ( !m_connection )
        emit newConnection( m_connection = new ITunesConnection );

    ITunesDictionaryHelper dict( info );
    State const previousState = m_state;
    m_state = dict.state;
    
    if ( m_state == Paused )
        m_connection->pause();
    else if ( m_state == Stopped )
        m_connection->stop();
    else if ( m_state == Playing )
    {
        QString output = m_currentTrackScript.exec();

        qDebug() << output;

        if ( !output.isEmpty() )
        {
            QTextStream s( &output, QIODevice::ReadOnly | QIODevice::Text );

            QString artist = s.readLine();
            QString albumArtist = s.readLine();
            QString album = s.readLine();
            QString track = s.readLine();
            QString duration = s.readLine();
            QString path = s.readLine();
            QString pid = s.readLine();
            bool podcast = s.readLine() == "true";
            QString videoKind = s.readLine();
            bool video = videoKind != "none" && videoKind != "music video";

            // if the track is restarted it has the same pid
            if ( m_previousPid == pid && previousState == Paused )
                m_connection->resume();
            else
            {
                MutableTrack t;
                t.setArtist( artist );
                t.setAlbumArtist( albumArtist );
                t.setTitle( track );
                t.setAlbum( album );
                t.setDuration( duration.toInt() );
                t.setUrl( QUrl::fromLocalFile( path ) );
                t.setPodcast( podcast );
                t.setVideo( video );
                m_connection->start( t );
            }

            m_previousPid = pid;
        }
        else
            m_connection->stop();
    }
    else
          qWarning() << "Unknown state.";

}


bool //static
ITunesListener::iTunesIsPlaying()
{
    const char* code = "tell application \"iTunes\" to if running then return player state is playing";
    return AppleScript( code ).exec() == "true";
}


void
ITunesListener::setupCurrentTrack()
{  
    if ( !iTunesIsPlaying() )
        return;

    QString output = m_currentTrackScript.exec();

    qDebug() << output;

    if ( !output.isEmpty() )
    {
        QTextStream s( &output, QIODevice::ReadOnly | QIODevice::Text );

        QString artist = s.readLine();
        QString albumArtist = s.readLine();
        QString album = s.readLine();
        QString track = s.readLine();
        QString duration = s.readLine();
        QString path = s.readLine();
        QString persistentID = s.readLine();
        bool podcast = s.readLine() == "true";
        QString videoKind = s.readLine();
        bool video = videoKind != "none" && videoKind != "music video";

        m_previousPid = persistentID;

        MutableTrack t;
        t.setArtist( artist );
        t.setAlbumArtist( albumArtist );
        t.setTitle( track );
        t.setAlbum( album );
        t.setDuration( duration.toInt() );
        t.setUrl( QUrl::fromLocalFile( path ) );
        t.setPodcast( podcast );
        t.setVideo( video );

        if ( !m_connection )
            emit newConnection( m_connection = new ITunesConnection );

        m_connection->start( t );
    }
}
