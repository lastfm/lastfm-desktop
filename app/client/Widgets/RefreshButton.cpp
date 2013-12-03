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

#include <QPainter>

#include "RefreshButton.h"

RefreshButton::RefreshButton(QWidget *parent) :
    QPushButton(parent), m_pixmap( ":/scrobbles_refresh.png" )
{
}

void
RefreshButton::paintEvent( QPaintEvent* e )
{
    QPushButton::paintEvent( e );

    QPainter p( this );
    QFontMetrics fm( font() );
    p.drawPixmap( rect().center() - QPoint( ( fm.width( text() ) / 2 ) + m_pixmap.width() + 10, ( m_pixmap.height() / 2 ) - 1 ), m_pixmap );
}
