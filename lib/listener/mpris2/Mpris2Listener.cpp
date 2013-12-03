/*
   Copyright (C) 2013 John Stamp <jstamp@mehercule.net>

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

#include "Mpris2Listener.h"
#include "Mpris2Service.h"
#include "../PlayerConnection.h"

#include <QDBusConnectionInterface>
#include <QRegExp>


class Mpris2Connection : public PlayerConnection
{
public:
    Mpris2Connection() : PlayerConnection( "mpris2", "" )
    {}

    void start( const Track& t )
    {
        MutableTrack mt( t );
        mt.setSource( Track::Player );
        mt.setExtra( "playerId", id() );
        mt.stamp();
        handleCommand( CommandStart, t );
    }

    void pause() { handleCommand( CommandPause ); }
    void resume() { handleCommand( CommandResume ); }
    void stop() { handleCommand( CommandStop ); }
};


Mpris2Listener::Mpris2Listener( QObject * parent ) : QObject( parent )
{

    stop();

    QStringList serviceNames = QDBusConnection::sessionBus().interface()->registeredServiceNames().value().filter(QLatin1String("org.mpris.MediaPlayer2."));
    foreach( const QString& name, serviceNames )
    {
        if ( !name.startsWith( "org.mpris.MediaPlayer2.") ||
             name.endsWith( "lastfm-scrobbler" ) )
            continue;
        addService( name );
    }
    connect( QDBusConnection::sessionBus().interface(),
            SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this,
            SLOT(onServiceOwnerChanged(QString,QString,QString)));
}

Mpris2Listener::~Mpris2Listener()
{
}


void
Mpris2Listener::createConnection()
{
    if ( !m_connection )
        emit newConnection( m_connection = new Mpris2Connection );
}


void
Mpris2Listener::addService( const QString& name )
{
    if ( m_services.keys().contains( name ) )
        return;

    Mpris2Service* service = new Mpris2Service( name, this );
    m_services.insert( name, service );
    connect( service,
             SIGNAL( stateChanged( const QString& )),
             this,
             SLOT(onChangedState( const QString& )) );
}


void
Mpris2Listener::stop()
{
    if ( m_connection )
        m_connection->stop();
    m_currentService = QString();
    m_lastPlayerState = "Stopped";
    m_lastTrack = lastfm::Track();
}


void
Mpris2Listener::removeService(const QString& name )
{
    if ( m_services.keys().contains( name ) )
    {
        if ( name == m_currentService )
            stop();
        Mpris2Service * service = m_services.take( name );
        delete service;
    }
}


void
Mpris2Listener::onServiceOwnerChanged( const QString& name, const QString& oldOwner, const QString& newOwner )
{
    if ( !name.startsWith( "org.mpris.MediaPlayer2.") ||
         name.endsWith( "lastfm-scrobbler" ) )
        return;

    if ( oldOwner.isEmpty() && !newOwner.isEmpty() )
        addService( name );
    else if ( newOwner.isEmpty() )
        removeService( name );
}


void
Mpris2Listener::onChangedState( const QString& state )
{
    Mpris2Service* service = static_cast<Mpris2Service*>(sender());

    // Once a player enters the Playing state, we keep monitoring it until it's
    // Stopped.  That way players running simultaneously don't fight for the
    // scrobbler's attention.
    if ( m_currentService.isEmpty() && state == "Playing" )
        m_currentService = service->name();

    if ( service->name() == m_currentService )
    {
        if ( state == "Playing" )
        {
            MutableTrack t;
            t.setTitle( service->title() );
            t.setArtist( service->artist() );
            if ( service->length() == 0 )
                t.setDuration( 320 );
            else
                t.setDuration( service->length() );
            if ( !service->url().isEmpty() )
                t.setUrl( service->url() );

            // Let PlaybackControlsWidget & ProgressBar know the friendly app name
            t.setExtra( "playerName", service->identity() );

            // We also want to add info from the optional DesktopEntry property
            // and a hint from the service name itself.  This will help
            // PlaybackControlsWidget find the right application icon to use.
            if ( !service->desktopEntry().isEmpty() )
                t.setExtra( "desktopEntry", service->desktopEntry() );

            QRegExp rx( "^org\\.mpris\\.MediaPlayer2\\.([^.]+)" );
            if ( rx.indexIn( service->name() ) >= 0 )
                t.setExtra( "serviceName", rx.cap( 1 ) );

            if ( m_lastPlayerState == "Paused" && t == m_lastTrack )
            {
                m_connection->resume();
            }
            else
            {
                m_connection->start( t );
                m_lastTrack = t;
            }
        }
        else if ( state == "Paused" )
        {
            m_connection->pause();
        }
        else if ( state == "Stopped" )
        {
            stop();
        }
        m_lastPlayerState = state;
    }
}
