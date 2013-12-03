/*
   Copyright 2010-2012 Last.fm Ltd.
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

#include "PointyArrow.h"
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QTimeLine>
#include <QApplication>
#include <QDesktopWidget>

PointyArrow::PointyArrow()
{
    setAttribute( Qt::WA_TranslucentBackground );
    setWindowFlags( Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
    
    m_timeline = new QTimeLine( 1000 );


    connect( m_timeline, SIGNAL( frameChanged( int )), SLOT( onFrameChanged( int )));
    connect( m_timeline, SIGNAL( finished()), SLOT( onFinished()));
}

PointyArrow::~PointyArrow()
{
    disconnect( m_timeline, 0, this, 0 );
    delete m_timeline;
}

void 
PointyArrow::paintEvent( QPaintEvent* /*event*/ )
{
    QPainter p( this );
    p.drawPixmap( QPoint( 0, 0 ), m_pm );
}

void 
PointyArrow::onFrameChanged( int frame )
{
    if( m_currentDirection == DirectionUp ||
        m_currentDirection == DirectionDown ) 
    {
        move( geometry().left(), frame );
    } else {
        move( frame, geometry().top() );
    }
}

void 
PointyArrow::onFinished()
{
    m_timeline->toggleDirection();
    m_timeline->start();
}

void
PointyArrow::pointAt( const QPoint& point )
{ 
    m_timeline->stop();
    QRect availRect = qApp->desktop()->availableGeometry( point );
    QRect screenRect = qApp->desktop()->screenGeometry( point );

    const bool autoHideSysTray = (availRect == screenRect);

    QRect taskbarRect;
    if( !autoHideSysTray ) {
        taskbarRect = (QRegion(screenRect) - QRegion(availRect)).rects().first();
    }

    if( point.x() < availRect.left() ) {
        //taskbar is left
        m_currentDirection = DirectionLeft;
        m_pm = QPixmap( ":/pointyarrow.png" ).transformed(QTransform().rotate(270.0));
        resize( m_pm.size());
        move( taskbarRect.right(), point.y() - (height() / 2.0f));
        m_timeline->setFrameRange( geometry().left(), geometry().left() + 40 );
    } else if( point.y() < availRect.top() ) {
        //taskbar is top
        m_currentDirection = DirectionUp;
        m_pm = QPixmap( ":/pointyarrow.png" );
        resize( m_pm.size());
        move( point.x() - (width() / 2.0f), taskbarRect.bottom() );
        m_timeline->setFrameRange( geometry().top(), geometry().top() + 40 );
    } else if( point.x() > availRect.right() ) {
        //taskbar is right
        m_currentDirection = DirectionRight;
        m_pm = QPixmap( ":/pointyarrow.png" ).transformed(QTransform().rotate(90.0));
        resize( m_pm.size());
        move( taskbarRect.left()-width(), point.y() - (height() / 2.0f));
        m_timeline->setFrameRange( geometry().left(), geometry().left() - 40 );
    } else {
        //presume that taskbar is at the bottom
        m_currentDirection = DirectionDown;
        m_pm = QPixmap( ":/pointyarrow.png" ).transformed(QTransform().rotate(180.0));
        resize( m_pm.size());
        move( point.x() - (width() / 2.0f), taskbarRect.top()-height() );
        m_timeline->setFrameRange( geometry().top(), geometry().top() - 40 );
    }

    m_timeline->setCurveShape( QTimeLine::EaseInOutCurve );
    m_timeline->start();
    show();
}
