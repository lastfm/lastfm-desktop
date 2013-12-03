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

#include "../../Application.h"
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
#include "lib/listener/legacy/LegacyPlayerListener.h"
#else
#include "lib/listener/mpris2/Mpris2Listener.h"
#endif
#include "lib/listener/PlayerConnection.h"
#include "lib/listener/PlayerListener.h"
#include "lib/listener/PlayerMediator.h"
#include "../MediaDevices/DeviceScrobbler.h"
#include "../RadioService/RadioService.h"
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
    qRegisterMetaType<Track>("Track");

/// mediator
    m_mediator = new PlayerMediator(this);
    connect( m_mediator, SIGNAL(activeConnectionChanged( PlayerConnection* )), SLOT(setConnection( PlayerConnection* )) );
    
/// listeners
    try{
#ifdef Q_OS_MAC
        ITunesListener* itunes = new ITunesListener(m_mediator);
        connect(itunes, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));

        SpotifyListenerMac* spotify = new SpotifyListenerMac(m_mediator);
        connect(spotify, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
#endif
#ifdef Q_OS_WIN
        SpotifyListenerWin* spotify = new SpotifyListenerWin(m_mediator);
        connect(spotify, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
#endif

        QObject* o = new PlayerListener(m_mediator);
        connect(o, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        o = new LegacyPlayerListener(m_mediator);
        connect(o, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
#else
        Mpris2Listener* mpris2 = new Mpris2Listener(m_mediator);
        connect(mpris2, SIGNAL(newConnection(PlayerConnection*)), m_mediator, SLOT(follow(PlayerConnection*)));
        mpris2->createConnection();
#endif
    }
    catch(std::runtime_error& e){
        qWarning() << e.what();
        //TODO user visible warning
    }

    m_mediator->follow( new RadioConnection( this ) );


    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );
    resetScrobbler();
}

ScrobbleService&
ScrobbleService::instance()
{
    static ScrobbleService s;
    return s;
}


bool
ScrobbleService::isDirExcluded( const lastfm::Track& track )
{
    if ( track.source() == lastfm::Track::LastFmRadio )
        return false;

    QString pathToTest = track.url().toLocalFile();

#ifdef Q_OS_WIN
    pathToTest = pathToTest.toLower();
#endif

    if ( pathToTest.isEmpty() )
        return false;

    unicorn::UserSettings us;
    QStringList exculsionDirs = us.value( "ExclusionDirs" ).toStringList();

    foreach ( QString bannedPath, exculsionDirs )
    {
        bannedPath = QDir( bannedPath ).absolutePath();
#ifdef Q_OS_WIN
        bannedPath = bannedPath.toLower();
#endif

        qDebug() << pathToTest << bannedPath;

        // Try and match start of given path with banned dir
        if ( pathToTest.startsWith( bannedPath ) )
        {
            // Found, this path is from a banned dir
            return true;
        }
    }

    // The path wasn't found in exclusions list
    return false;
}

bool
ScrobbleService::scrobblableTrack( const lastfm::Track& track ) const
{
    unicorn::UserSettings userSettings;

    return userSettings.scrobblingOn()
            && ( track.extra( "playerId" ) != "spt" && track.extra( "playerId" ) != "mpris2" )
            && !track.artist().isNull()
            && ( userSettings.podcasts() || !track.isPodcast() )
            && !track.isVideo()
            && !isDirExcluded( track );
}

bool
ScrobbleService::scrobblingOn() const
{
    return scrobblableTrack( m_currentTrack );
}

void
ScrobbleService::scrobbleSettingsChanged()
{
    if ( m_watch )
    {
        ScrobblePoint timeout( ( m_currentTrack.duration() * unicorn::UserSettings().scrobblePoint() ) / 100.0 );
        timeout.setEnforceScrobbleTimeMax( unicorn::UserSettings().enforceScrobbleTimeMax() );
        m_watch->setScrobblePoint( timeout );
    }

    bool scrobblingOn = scrobblableTrack( m_currentTrack );

    // Check if the current track now meets the requirements
    // and scrobble it straight away if so
    if ( m_as
         && m_watch
         && m_watch->elapsed() >= (m_watch->scrobblePoint() * 1000)
         && m_currentTrack.scrobbleStatus() == Track::Null
         && scrobblingOn )
        m_as->cache( m_currentTrack );

    emit scrobblingOnChanged( scrobblingOn );
}

void
ScrobbleService::submitCache()
{
    if ( m_as ) m_as->submit();
}

void 
ScrobbleService::onSessionChanged( const unicorn::Session& )
{
    resetScrobbler();
}

void
ScrobbleService::resetScrobbler()
{
    if ( !aApp->currentSession().user().name().isEmpty()
         && aApp->currentSession().user().name() != m_currentUsername)
    {
        // only create the scrobble cache, etc if we have a user
        // we won't have a user during the first run wizard

        m_currentUsername = aApp->currentSession().user().name();

        /// audioscrobbler
        delete m_as;
        m_as = new Audioscrobbler( "ass" );
        connect( m_as, SIGNAL(scrobblesCached(QList<lastfm::Track>)), SIGNAL(scrobblesCached(QList<lastfm::Track>)));
        connect( m_as, SIGNAL(scrobblesSubmitted(QList<lastfm::Track>)), SIGNAL(scrobblesSubmitted(QList<lastfm::Track>)));

        /// DeviceScrobbler
        delete m_deviceScrobbler;
        m_deviceScrobbler = new DeviceScrobbler( this );
        connect( m_deviceScrobbler, SIGNAL(foundScrobbles(QList<lastfm::Track>)), SLOT(onFoundScrobbles(QList<lastfm::Track>)));
        connect( m_deviceScrobbler, SIGNAL(foundScrobbles(QList<lastfm::Track>)), SIGNAL(foundIPodScrobbles(QList<lastfm::Track>)));

        // Do this a bit later to as it's nicer for the user and
        // it gives the main window time to be diplayed on boot
        QTimer::singleShot( 3000, m_deviceScrobbler, SLOT(checkCachedIPodScrobbles()) );
    }
}

void
ScrobbleService::onFoundScrobbles( QList<lastfm::Track> tracks )
{
    m_as->cacheBatch( tracks );
    m_as->submit();
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
    connect(c, SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), this, SLOT(onTrackStarted(lastfm::Track,lastfm::Track)), Qt::QueuedConnection);
    connect(c, SIGNAL(paused()), this, SLOT(onPaused()), Qt::QueuedConnection);
    connect(c, SIGNAL(resumed()), this, SLOT(onResumed()), Qt::QueuedConnection);
    connect(c, SIGNAL(stopped()), this, SLOT(onStopped()), Qt::QueuedConnection);
    connect(c, SIGNAL(bootstrapReady(QString)), SIGNAL( bootstrapReady(QString)));

    m_connection = c;

    if(c->state() == Playing || c->state() == Paused)
        c->forceTrackStarted(Track());

    if( c->state() == Paused )
        c->forcePaused();
}

void
ScrobbleService::onTrackStarted( const lastfm::Track& t, const lastfm::Track& ot )
{
    Q_ASSERT(m_connection);

    //TODO move to playerconnection
    if(t.isNull())
    {
        qWarning() << "Can't start null track!";
        return;
    }

    Track oldtrack = ot.isNull() ? m_currentTrack : ot;

    if ( unicorn::UserSettings().scrobblePoint() == 100.0 && !oldtrack.isNull() )
    {
        // was the last track at 100%? Should we scrobble it?

        if ( m_watch )
        {
            /** At this point, we can't tell for ceratin if a track was played until the end.
             * The next lowest percentage the user can set is 95% which means a 30 second track
             * will scrobble at 28.5 seconds. As long as we're above this, I think we're fine.
             * Here we'rechecking if the track got within 1 second of the end point At 30 seconds.
             * At 30 seconds this equates to 96.66%, which is tolerable. Assuming an average 3
             * minute track length, the actual percentage we will at is 99.44% which is close
             * enough to 100% for me. As this is timing based, things can go slightly off, it's
             * still possible that scrobbles will be missed, but these are the perils of setting
             * your scrobble point to 100%.
             * TODO: really the plugins should be able to tell us if the track got to 100% and
             * we just use that, but I'm not sure if that's possible and that would be a rather
             * big change. This should be enough for now.
             */
            if ( !m_watch->paused() // the watch is running (if we scrobble after the user paused and skipped, it would give away we're not actually scrobbling at 100%)
                 && oldtrack.duration() == m_watch->duration() // the duration of the track and the stop watch duration is the same (double checking it's the same track)
                 && m_watch->elapsed() > (m_watch->duration() * 1000) - 1000 ) // it's within a second so assume it got to 100% (see above)
                onScrobble(); // we should scrobble the last track
        }

    }

    m_state = Playing;
    m_currentTrack = t;

    ScrobblePoint timeout( ( m_currentTrack.duration() * unicorn::UserSettings().scrobblePoint() ) / 100.0 );
    timeout.setEnforceScrobbleTimeMax( unicorn::UserSettings().enforceScrobbleTimeMax() );
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
    if ( m_state == Unknown ) return;

    m_state = Paused;

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
    if ( m_state == Unknown ) return;

    m_state = Stopped;

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
    if ( m_state == Unknown ) return;

    m_state = Playing;

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

    if( m_as
            && scrobblableTrack( m_currentTrack )
            && m_currentTrack.scrobbleStatus() == Track::Null )
        m_as->cache( m_currentTrack );
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

