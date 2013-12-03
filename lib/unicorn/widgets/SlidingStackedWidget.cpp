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

#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include "SlidingStackedWidget.h"

unicorn::SlidingStackedWidget::SlidingStackedWidget( QWidget* parent )
    : QStackedWidget(parent),
    m_speed( 200 ),
    m_animationtype( QEasingCurve::OutQuart ),
    m_vertical( false ),
    m_now( 0 ),
    m_next( 0 ),
    m_pnow( QPoint(0,0) ),
    m_active( false ),
    m_index( 0 )
{
}

void
unicorn::SlidingStackedWidget::setAnimation(enum QEasingCurve::Type animationtype)
{
        m_animationtype = animationtype;
}

void
unicorn::SlidingStackedWidget::slide( int index )
{
    m_index = index;

    if ( !m_active )
    {
        if ( index > count() - 1 )
            index = (index) % count();
        else
            index = ( index + count() ) % count();

        slideWidget( widget( index ) );
    }
}

void
unicorn::SlidingStackedWidget::slideWidget( QWidget* newwidget )
{
    m_active=true;

    enum t_direction directionhint;
    int now=currentIndex();		//currentIndex() is a function inherited from QStackedWidget
    int next=indexOf(newwidget);

    if ( now == next )
    {
        m_active=false;
        emit currentChanged( currentIndex() );
        return;
    }
    else if ( now < next )
    {
        directionhint = m_vertical ? TOP2BOTTOM : RIGHT2LEFT;
    }
    else
    {
        directionhint = m_vertical ? BOTTOM2TOP : LEFT2RIGHT;
    }

    //NOW....
    //calculate the shifts

    int offsetx=frameRect().width(); //inherited from mother
    int offsety=frameRect().height();//inherited from mother

    //the following is important, to ensure that the new widget
    //has correct geometry information when sliding in first time
    widget(next)->setGeometry ( 0,  0, offsetx, offsety );

    if (directionhint==BOTTOM2TOP)
    {
        offsetx=0;
        offsety=-offsety;
    }
    else if (directionhint==TOP2BOTTOM)
    {
        offsetx=0;
        //offsety=offsety;
    }
    else if (directionhint==RIGHT2LEFT)
    {
        offsetx=-offsetx;
        offsety=0;
    }
    else if (directionhint==LEFT2RIGHT)
    {
        //offsetx=offsetx;
        offsety=0;
    }
    //re-position the next widget outside/aside of the display area
    QPoint pnext=widget(next)->pos();
    QPoint pnow=widget(now)->pos();
    m_pnow=pnow;

    widget(next)->move(pnext.x()-offsetx,pnext.y()-offsety);
    //make it visible/show
    widget(next)->show();
    widget(next)->raise();

    //animate both, the now and next widget to the side, using animation framework
    QPropertyAnimation *animnow = new QPropertyAnimation(widget(now), "pos");

    animnow->setDuration(m_speed);
    animnow->setEasingCurve(m_animationtype);
    animnow->setStartValue(QPoint(pnow.x(), pnow.y()));
    animnow->setEndValue(QPoint(offsetx+pnow.x(), offsety+pnow.y()));
    QPropertyAnimation *animnext = new QPropertyAnimation(widget(next), "pos");
    animnext->setDuration(m_speed);
    animnext->setEasingCurve(m_animationtype);
    animnext->setStartValue(QPoint(-offsetx+pnext.x(), offsety+pnext.y()));
    animnext->setEndValue(QPoint(pnext.x(), pnext.y()));

    QParallelAnimationGroup *animgroup = new QParallelAnimationGroup;

    animgroup->addAnimation(animnow);
    animgroup->addAnimation(animnext);

    QObject::connect(animgroup, SIGNAL(finished()),this,SLOT(animationDoneSlot()));
    m_next=next;
    m_now=now;
    m_active=true;
    animgroup->start();
}


void
unicorn::SlidingStackedWidget::animationDoneSlot(void)
{
    setCurrentIndex( m_next );
    widget(m_now)->hide();
    widget(m_now)->move(m_pnow);
    m_active = false;

    // animate again if they changed index while we were animating
    if ( m_index != currentIndex() )
        slide( m_index );
    else
        emit animationFinished();
}
