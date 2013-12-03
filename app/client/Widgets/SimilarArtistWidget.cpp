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

#include "SimilarArtistWidget.h"

SimilarArtistWidget::SimilarArtistWidget(QWidget *parent) :
    HttpImageWidget(parent)
{
}

void
SimilarArtistWidget::setArtist( const QString& artist )
{
    m_artist = artist;
}

void
SimilarArtistWidget::paintEvent( QPaintEvent* event )
{
    HttpImageWidget::paintEvent( event );

    QPainter p;
    p.begin( this );

    QTextOption to;

    to.setAlignment( Qt::AlignBottom | Qt::AlignLeft );

    QFontMetrics fm( font() );

    QRect rect = contentsRect().adjusted( -3, 0, 13, 22 );

    p.drawText( rect, fm.elidedText( m_artist, Qt::ElideRight, rect.width() ), to );

    p.end();
}
