/*
   Copyright 2005-2010 Last.fm Ltd. 
      - Primarily authored by Jono Cole and Michael Coffey

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

#include "ScrobbleService.h"
#include <lastfm/ws.h>

#ifdef QT_DBUS_LIB
#include "lib/listener/DBusListener.h"
#endif
#include "lib/listener/PlayerConnection.h"
#include "lib/listener/PlayerListener.h"
#include "lib/listener/PlayerMediator.h"
#include "../MediaDevices/DeviceScrobbler.h"
#include "../RadioService/RadioConnection.h"
#include "StopWatch.h"
#ifdef Q_WS_MAC
#include "lib/listener/mac/SpotifyListener.h"
#include "lib/listener/mac/ITunesListener.h"
#endif
#ifdef Q_OS_WIN
#include "lib/listener/win/SpotifyListener.h"
#endif

ScrobbleService::ScrobbleService()
{
/// mediator
    m_mediator = new PlayerMediator(this);
    connect( m_mediator, SIGNAL(activeConnectionChanged( PlayerConnection* )), SLOT(setConnection( PlayerConnection* )) );
    
/// listeners
    try{
#ifdef Q_OS_MAC
        ITunesListener* itunes = new ITunesListener(m_mediator);
        connect(itunes, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
        itunes->start();

        SpotifyListenerMac* spotify = new SpotifyListenerMac(m_mediator);
        connect(spotify, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
#endif
#ifdef Q_OS_WIN
        SpotifyListenerWin* spotify = new SpotifyListenerWin(m_mediator);
        connect(spotify, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
#endif

        QObject* o = new PlayerListener(m_mediator);
        connect(o, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));

#ifdef QT_DBUS_LIB
        DBusListener* dbus = new DBusListener(mediator);
        connect(dbus, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
#endif
    }
    catch(std::runtime_error& e){
        qWarning() << e.what();
        //TODO user visible warning
    }

    m_mediator->follow( new RadioConnection( this ) );

    resetScrobbler();
}


bool
ScrobbleService::scrobblableTrack( const lastfm::Track& track ) const
{
    return unicorn::UserSettings().value( "scrobblingOn", true ).toBool()
            && track.extra( "playerId" ) != "spt"
            && !track.artist().isNull()
            && ( unicorn::UserSettings().value( "podcasts", true ).toBool() || !track.isPodcast() )
            && !track.isVideo();
}

bool
ScrobbleService::scrobblingOn() const
{
    return scrobblableTrack( m_trackToScrobble );
}

void
ScrobbleService::scrobbleSettingsChanged()
{
    bool scrobblingOn = scrobblableTrack( m_trackToScrobble );

    if ( m_as
         && m_watch
         && m_watch->scrobbled()
         && m_trackToScrobble.scrobbleStatus() == Track::Null
         && scrobblingOn )
        m_as->cache( m_trackToScrobble );

    emit scrobblingOnChanged( scrobblingOn );
}

void
ScrobbleService::submitCache()
{
    if ( m_as ) m_as->submit();
}

void 
ScrobbleService::onSessionChanged( unicorn::Session* ) 
{
    resetScrobbler();
}

void
ScrobbleService::resetScrobbler()
{
/// audioscrobbler
    if( m_as )
        delete m_as;

    m_as = new Audioscrobbler( "ass" );
    connect( m_as, SIGNAL(scrobblesCached(QList<lastfm::Track>)), SIGNAL(scrobblesCached(QList<lastfm::Track>)));
    connect( m_as, SIGNAL(scrobblesSubmitted(QList<lastfm::Track>)), SIGNAL(scrobblesSubmitted(QList<lastfm::Track>)));

/// DeviceScrobbler
    if( m_deviceScrobbler )
        delete m_deviceScrobbler;

    m_deviceScrobbler = new DeviceScrobbler;
    connect( m_deviceScrobbler, SIGNAL(foundScrobbles( QList<lastfm::Track>, QString)), this, SIGNAL( foundIPodScrobbles(QList<lastfm::Track>, QString) ));
    connect( m_deviceScrobbler, SIGNAL(foundScrobbles( QList<lastfm::Track>, QString )), m_as, SLOT( cacheBatch( QList<lastfm::Track>, QString)));
    m_deviceScrobbler->checkCachedIPodScrobbles();
}

void
ScrobbleService::setConnection(PlayerConnection*c)
{
    if( m_connection )
    {
        // disconnect from all the objects that we connect to below
        disconnect( m_connection, 0, this, 0);
        if(m_watch)
            m_connection->setElapsed(m_watch->elapsed());
    }

    //
    connect(c, SIGNAL(trackStarted(Track, Track)), SLOT(onTrackStarted(Track, Track)));
    connect(c, SIGNAL(paused()), SLOT(onPaused()));
    connect(c, SIGNAL(resumed()), SLOT(onResumed()));
    connect(c, SIGNAL(stopped()), SLOT(onStopped()));

    //connect(c, SIGNAL(trackStarted(Track, Track)), SIGNAL(trackStarted(Track, Track)));
    connect(c, SIGNAL(resumed()), SIGNAL(resumed()));
    connect(c, SIGNAL(paused()), SIGNAL(paused()));
    connect(c, SIGNAL(stopped()), SIGNAL(stopped()));
    connect(c, SIGNAL(bootstrapReady(QString)), SIGNAL( bootstrapReady(QString)));

    m_connection = c;

    if(c->state() == Playing || c->state() == Paused)
        c->forceTrackStarted(Track());

    if( c->state() == Paused )
        c->forcePaused();
}

void
ScrobbleService::onTrackStarted( const Track& t, const Track& ot )
{
    Q_ASSERT(m_connection);

    state = Playing;

    Track oldtrack = ot.isNull() ? m_currentTrack : ot;

    //TODO move to playerconnection
    if(t.isNull())
    {
        qWarning() << "Can't start null track!";
        return;
    }

    m_currentTrack = t;

    double trackLengthPercent = unicorn::UserSettings().value( "scrobblePoint", 50 ).toDouble() / 100.0;

    //This is to prevent the next track being scrobbled
    //instead of the track just listened
    if ( trackLengthPercent == 100 && !oldtrack.isNull() )
    {
        m_trackToScrobble = oldtrack;
    }
    else
    {
        m_trackToScrobble = t;
    }

    ScrobblePoint timeout( m_currentTrack.duration() * trackLengthPercent );
    delete m_watch;
    m_watch = new StopWatch(m_currentTrack.duration(), timeout);
    m_watch->start();

    connect( m_watch, SIGNAL(scrobble()), SLOT(onScrobble()));
    connect( m_watch, SIGNAL(paused(bool)), SIGNAL(paused(bool)));
    connect( m_watch, SIGNAL(frameChanged( int )), SIGNAL(frameChanged( int )));
    connect( m_watch, SIGNAL(timeout()), SIGNAL(timeout()));

    qDebug() << "********** AS = " << m_as;
    if( m_as )
    {
        m_as->submit();

        if ( scrobblableTrack( t ) )
        {
            qDebug() << "************** Now Playing..";
            m_as->nowPlaying( t );
        }
    }

    emit trackStarted( t, oldtrack );
}

void
ScrobbleService::onPaused()
{
    // We can sometimes get a stopped before a play when the
    // media player is playing before the scrobbler is started
    if ( state == Unknown ) return;

    state = Paused;

    //m_currentTrack.removeNowPlaying();

    Q_ASSERT(m_connection);
    Q_ASSERT(m_watch);
    if(m_watch) m_watch->pause();
    
    emit paused();
}

void
ScrobbleService::onStopped()
{
    // We can sometimes get a stopped before a play when the
    // media player is playing before the scrobbler is started
    if ( state == Unknown ) return;

    state = Stopped;

    Q_ASSERT(m_watch);
    Q_ASSERT(m_connection);
        
    delete m_watch;
    if( m_as )
        m_as->submit();

    emit stopped();
}

void
ScrobbleService::onResumed()
{
    // We can sometimes get a stopped before a play when the
    // media player is playing before the scrobbler is started
    if ( state == Unknown ) return;

    state = Playing;

    Q_ASSERT(m_watch);
    Q_ASSERT(m_connection);

    //m_currentTrack.updateNowPlaying( m_currentTrack.duration() - (m_watch->elapsed()/1000) );

    if (m_watch)
        m_watch->resume();

    emit resumed();
}

void
ScrobbleService::onScrobble()
{
    Q_ASSERT(m_connection);

    if( m_as && scrobblableTrack( m_trackToScrobble ) )
        m_as->cache( m_trackToScrobble );
}

void 
ScrobbleService::handleIPodDetectedMessage( const QStringList& message )
{
    m_deviceScrobbler->iPodDetected( message );
}

void 
ScrobbleService::handleTwiddlyMessage( const QStringList& message )
{
    m_deviceScrobbler->handleMessage( message );
}


ScrobbleService::~ScrobbleService()
{    
}
