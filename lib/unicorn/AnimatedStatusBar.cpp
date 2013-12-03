/*
   Copyright 2009 Last.fm Ltd.
      - Primarily authored by Jono Cole

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

#include "AnimatedStatusBar.h"

#include <QTimeLine>
#include "qtwin.h"
#include <QDebug>

AnimatedStatusBar::AnimatedStatusBar( QWidget* parent )
                  :QStatusBar( parent )
{
    m_timeline = new QTimeLine( 50, this );
    m_timeline->setUpdateInterval( 20 );
    m_timeline->setCurveShape( QTimeLine::EaseInCurve );
    connect( m_timeline, SIGNAL( frameChanged(int)), SLOT(onFrameChanged(int)));
    connect( m_timeline, SIGNAL( finished()), SLOT(onFinished()));
}


void
AnimatedStatusBar::showAnimated()
{
    if( isVisible() && height() > 0 ) return;
    
    window()->setMinimumHeight( window()->height());
    m_timeline->setFrameRange( 0, sizeHint().height());
    m_timeline->setDirection( QTimeLine::Forward );
    setFixedHeight( 0 );
    show();
    m_windowHeight = window()->height();
    m_timeline->start();
}


void 
AnimatedStatusBar::hideAnimated()
{
    if( !isVisible() || height() == 0 ) return;
    
    window()->setMaximumHeight( window()->height());
    m_timeline->setFrameRange( 0, sizeHint().height());
    m_timeline->setDirection( QTimeLine::Backward );
    
    m_windowHeight = window()->height();
    m_timeline->start();
}

void 
AnimatedStatusBar::onFinished()
{
    if( m_timeline->direction() == QTimeLine::Backward ) {
        hide();
    }

    setMaximumHeight( QWIDGETSIZE_MAX );
    setMinimumHeight( 0 );
    window()->setMinimumHeight( 0 );
    window()->setMaximumHeight( QWIDGETSIZE_MAX );
}


void 
AnimatedStatusBar::onFrameChanged( int f )
{
    setFixedHeight( f );
    if( m_timeline->direction() == QTimeLine::Forward )
        window()->resize( window()->width(), m_windowHeight + f );
    else
        window()->resize( window()->width(), m_windowHeight - (sizeHint().height() - f ) );
}
