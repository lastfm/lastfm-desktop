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
#include "PlayerMediator.h"


PlayerMediator::PlayerMediator( QObject* parent )
              : QObject( parent )
{}


void
PlayerMediator::follow( PlayerConnection* connection )
{
    if (m_connections.contains( connection )) { qWarning() << "Already following:" << connection; return; }
    
    m_connections += connection;

    connect( connection, SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), SLOT(onActivity()) );
    connect( connection, SIGNAL(resumed()), SLOT(onActivity()) );
    connect( connection, SIGNAL(stopped()), SLOT(onActivity()) );
    connect( connection, SIGNAL(bootstrapReady(QString)), SLOT(onActivity()) );
    connect( connection, SIGNAL(destroyed()), SLOT(onDestroyed()) );

    assess( connection );
}


void
PlayerMediator::onActivity()
{
    PlayerConnection* connection = qobject_cast<PlayerConnection*>(sender());

    if (m_active == connection)
    {
        if (connection->state() == Stopped)
        {
            foreach (PlayerConnection* connection, m_connections)
                if (connection != m_active && assess( connection ))
                        return;
        }
    }
    else
        assess( connection );
}


bool
PlayerMediator::assess( PlayerConnection* connection )
{
    Q_ASSERT( connection );
    
    if (!m_active)
        goto set_active;
    
    if ( m_active->state() == Stopped || m_active->state() == Paused
         || connection->id() == "ass" ) // the radio connection steals from all other sources
    {
        switch ( connection->state() )
        {
            case Playing:
                goto set_active;

            case TuningIn:
            case Buffering:
                qWarning() << "Unsupported state for PlayerConnection";
                break;                

            default:
            case Stopped:
            case Paused:
                break;
        }
    }
    
    return false;
    
set_active:
    if (!connection)
        return false;

    m_active = connection;
    emit activeConnectionChanged( connection );

    return true;
}


void 
PlayerMediator::onDestroyed()
{
    m_connections.removeAll( (PlayerConnection*) this->sender() );
}
