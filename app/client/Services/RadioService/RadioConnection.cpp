/*
   Copyright 2011 Last.fm Ltd.
      - Primarily authored by Michael Coffey

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

#include "RadioConnection.h"
#include "RadioService.h"

RadioConnection::RadioConnection( QObject* parent )
    :PlayerConnection( "ass", "Last.fm Radio", parent )
{    
    connect( &RadioService::instance(), SIGNAL(trackSpooled(Track)), SLOT(onTrackSpooled(Track)));
    connect( &RadioService::instance(), SIGNAL(paused()), SIGNAL(paused()));
    connect( &RadioService::instance(), SIGNAL(resumed()), SIGNAL(resumed()));
    connect( &RadioService::instance(), SIGNAL(stopped()), SIGNAL(stopped()));
}


Track
RadioConnection::track() const
{
    return RadioService::instance().currentTrack();
}


State
RadioConnection::state() const
{
    return RadioService::instance().state();
}

void
RadioConnection::onTrackSpooled( const Track& track )
{
    MutableTrack mTrack( track );
    mTrack.setSource( Track::LastFmRadio );
    mTrack.setExtra( "playerId", m_id );
    mTrack.setExtra( "playerName", m_name );
    emit trackStarted( track, Track() );
}
