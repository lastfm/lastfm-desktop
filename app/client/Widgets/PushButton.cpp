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

#include <QPixmap>
#include <QPainter>
#include <QEvent>
#include <QStyle>

#include "PushButton.h"

PushButton::PushButton(QWidget *parent) :
    QPushButton(parent),
    m_hovered( false ),
    m_dark( false )
{
    setAttribute( Qt::WA_LayoutUsesWidgetRect );
    setAttribute( Qt::WA_Hover );
    setAttribute( Qt::WA_MacNoClickThrough );

    setCursor( Qt::PointingHandCursor );
}

bool
PushButton::dark() const
{
    return m_dark;
}

void
PushButton::setDark( bool dark )
{
    m_dark = dark;
    //style()->polish( this );
}

bool
PushButton::event( QEvent* e )
{
    switch ( e->type() )
    {
    case QEvent::HoverEnter:
        m_hovered = true;
        update();
        break;
    case QEvent::HoverLeave:
        m_hovered = false;
        update();
        break;
    default:
        break;
    }

    return QPushButton::event( e );
}

void
PushButton::paintEvent(QPaintEvent *)
{
    static QPixmap leftHoverLight( ":/button_LEFT_HOVER.png" );
    static QPixmap leftPressLight( ":/button_LEFT_PRESS.png" );
    static QPixmap leftRestLight( ":/button_LEFT_REST.png" );
    static QPixmap middleHoverLight( ":/button_MIDDLE_HOVER.png" );
    static QPixmap middlePressLight( ":/button_MIDDLE_PRESS.png" );
    static QPixmap middleRestLight( ":/button_MIDDLE_REST.png" );
    static QPixmap rightHoverLight( ":/button_RIGHT_HOVER.png" );
    static QPixmap rightPressLight( ":/button_RIGHT_PRESS.png" );
    static QPixmap rightRestLight( ":/button_RIGHT_REST.png" );

    static QPixmap leftHoverDark( ":/button_dark_LEFT_HOVER.png" );
    static QPixmap leftPressDark( ":/button_dark_LEFT_PRESS.png" );
    static QPixmap leftRestDark( ":/button_dark_LEFT_REST.png" );
    static QPixmap middleHoverDark( ":/button_dark_MIDDLE_HOVER.png" );
    static QPixmap middlePressDark( ":/button_dark_MIDDLE_PRESS.png" );
    static QPixmap middleRestDark( ":/button_dark_MIDDLE_REST.png" );
    static QPixmap rightHoverDark( ":/button_dark_RIGHT_HOVER.png" );
    static QPixmap rightPressDark( ":/button_dark_RIGHT_PRESS.png" );
    static QPixmap rightRestDark( ":/button_dark_RIGHT_REST.png" );

    QPixmap* leftHover = m_dark ? &leftHoverDark : &leftHoverLight;
    QPixmap* leftPress = m_dark ? &leftPressDark : &leftPressLight;
    QPixmap* leftRest = m_dark ? &leftRestDark : &leftRestLight;
    QPixmap* middleHover = m_dark ? &middleHoverDark : &middleHoverLight;
    QPixmap* middlePress = m_dark ? &middlePressDark : &middlePressLight;
    QPixmap* middleRest = m_dark ? &middleRestDark : &middleRestLight;
    QPixmap* rightHover = m_dark ? &rightHoverDark : &rightHoverLight;
    QPixmap* rightPress = m_dark ? &rightPressDark : &rightPressLight;
    QPixmap* rightRest = m_dark ? &rightRestDark : &rightRestLight;

    QPainter p( this );

    QRect middleRect = QRect( leftRest->width(), 0, rect().width() - leftRest->width() - rightRest->width(), 49 );

    if ( isDown() )
    {
        p.drawPixmap( rect().topLeft(), *leftPress );
        p.drawPixmap( middleRect, *middlePress );
        p.drawPixmap( rect().topLeft() + QPoint( rect().width() - rightPress->width(), 0 ), *rightPress );
    }
    else if ( m_hovered )
    {
        p.drawPixmap( rect().topLeft(), *leftHover );
        p.drawPixmap( middleRect, *middleHover );
        p.drawPixmap( rect().topLeft() + QPoint( rect().width() - rightHover->width(), 0 ), *rightHover );
    }
    else
    {
        p.drawPixmap( rect().topLeft(), *leftRest );
        p.drawPixmap( middleRect, *middleRest );
        p.drawPixmap( rect().topLeft() + QPoint( rect().width() - rightRest->width(), 0 ), *rightRest );
    }

    QFontMetrics fmText( font() );
    QRect textRect = contentsRect();
    p.drawText( textRect, Qt::AlignCenter, fmText.elidedText( text(), Qt::ElideRight, textRect.width() ) );
}
