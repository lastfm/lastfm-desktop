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
#include "StopWatch.h"
#include <QTimeLine>


StopWatch::StopWatch( uint duration, ScrobblePoint timeout )
    : m_duration( duration ), m_point( timeout ), m_scrobbled(false)
{    
    m_timeline = new QTimeLine( duration * 1000, this );
    m_timeline->setFrameRange( 0, duration * 2 );
    m_timeline->setEasingCurve( QEasingCurve::Linear );
    m_timeline->setUpdateInterval( 50 );
    m_timeline->setCurrentTime( 0 );

    connect( m_timeline, SIGNAL(finished()), SIGNAL(timeout()) );
    connect( m_timeline, SIGNAL(frameChanged(int)), SLOT(onFrameChanged(int)));
}

ScrobblePoint
StopWatch::scrobblePoint() const
{
    return m_point;
}

void
StopWatch::setScrobblePoint( const ScrobblePoint& timeout_in_seconds )
{
    m_point = timeout_in_seconds;
}

uint
StopWatch::duration() const
{
    return m_duration;
}

bool
StopWatch::scrobbled() const
{
    return m_scrobbled;
}

void
StopWatch::onFrameChanged( int /*frame*/ )
{
    emit frameChanged( m_timeline->currentTime() );

    if ( !m_scrobbled && elapsed() >= (m_point * 1000) )
    {
        emit scrobble();
        m_scrobbled = true;
    }
}

bool
StopWatch::paused()
{
    return (m_timeline->state() == QTimeLine::Paused);
}

void
StopWatch::start()
{
    m_timeline->start();
    emit paused( false );
}

void
StopWatch::pause()
{
    m_timeline->setPaused( true );
    emit paused( true );
}

void
StopWatch::resume()
{
    // Only resume if we are already running
    if ( m_timeline->state() == QTimeLine::Paused )
        m_timeline->resume();
    emit paused( false );
}

uint
StopWatch::elapsed() const
{
    return m_timeline->currentTime();
}
