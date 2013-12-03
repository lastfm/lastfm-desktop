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
#include <QDebug>

#include "ContextLabel.h"

ContextLabel::ContextLabel( QWidget* parent )
    :unicorn::Label( parent )
{
}

void
ContextLabel::paintEvent( QPaintEvent* event )
{
    QLabel::paintEvent( event );

    // draw the arrow on the context
    QPainter p;
    p.begin( this );

    static QPixmap arrow( ":/meta_context_arrow.png" );

    // these values match the ones in the stylesheet
    int topMargin = 20;
    int leftMargin = 20;

    if ( this->objectName() == "userBlurb" )
    {
        topMargin = 12;
        leftMargin = 0;
    }

    QPoint arrowPoint = QPoint( leftMargin + (( 126 + 10 - arrow.size().width() )  / 2 ), topMargin + 1 - arrow.size().height() );
    p.drawPixmap( arrowPoint, arrow );

    p.end();
}
