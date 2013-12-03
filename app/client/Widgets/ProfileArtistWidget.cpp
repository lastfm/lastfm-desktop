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

#include <QHBoxLayout>

#include <lastfm/XmlQuery.h>
#include <lastfm/Artist.h>
#include <lastfm/RadioStation.h>

#include "lib/unicorn/widgets/HttpImageWidget.h"
#include "lib/unicorn/widgets/Label.h"

#include "../Application.h"
#include "PlayableItemWidget.h"

#include "ProfileArtistWidget.h"


class PlaysLabel : public QLabel
{
public:
    explicit PlaysLabel( const QString& text, int plays, int maxPlays, QWidget* parent = 0 )
        :QLabel( text, parent ),
          m_plays( plays ),
          m_maxPlays( maxPlays )
    {}

private:
    void paintEvent( QPaintEvent* event )
    {
        QPainter p;
        p.begin( this );

        QFontMetrics fm( font() );

        p.setPen( QColor( 0xa6a6a6 ) );
        p.setBrush( QColor( 0xdedede ) );
        p.drawRoundedRect( rect().adjusted( 0, 0, -1, -1 ), 4, 4 );

        int indent = fm.width( tr( "%L1 play(s)", "", 999999 ).arg( 999999 ) );
        int chunk = ( (width() - indent ) * m_plays ) / m_maxPlays;
        int adjust = indent + chunk - width();
        p.setPen( QColor( 0x2a8bad ) );
        p.setBrush( QColor( 0x34bae8 ) );
        p.drawRoundedRect( rect().adjusted( 0, 0, adjust - 1, -1 ), 4, 4 );

        p.end();

        QLabel::paintEvent( event );
    }

private:
    int m_plays;
    int m_maxPlays;
};


ProfileArtistWidget::ProfileArtistWidget( const lastfm::XmlQuery& artist, int maxPlays, QWidget* parent)
    :QFrame( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );

    HttpImageWidget* artistImage = new HttpImageWidget( this );
    layout->addWidget( artistImage );
    artistImage->setObjectName( "artistImage" );

    QRegExp re( "/serve/(\\d*)s?/" );

    artistImage->loadUrl( artist["image size=medium"].text().replace( re, "/serve/\\1s/" ), HttpImageWidget::ScaleNone );
    artistImage->setHref( artist["url"].text() );

    QVBoxLayout* vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->setSpacing( 6 );
    layout->addLayout( vl, 1 );

    QHBoxLayout * hl = new QHBoxLayout();
    hl->setContentsMargins( 0, 0, 0, 0 );
    hl->setSpacing( 0 );

    unicorn::Label* artistName = new unicorn::Label( this );
    artistName->setTextFormat( Qt::RichText );
    artistName->setText( unicorn::Label::boldLinkStyle( unicorn::Label::anchor( artist["url"].text(), artist["name"].text() ), Qt::black ) );
    hl->addWidget( artistName, 1 );
    artistName->setObjectName( "artistName" );

    PlayableItemWidget* radio = new PlayableItemWidget( RadioStation::similar( Artist( artist["name"].text() ) ), tr( "%1 Radio" ).arg( artist["name"].text() ) );
    hl->addWidget( radio );
    radio->setObjectName( "radio" );

    vl->addLayout( hl );

    int playcount = artist["playcount"].text().toInt();
    PlaysLabel* plays = new PlaysLabel( tr( "%L1 play(s)", "", playcount ).arg( playcount ), playcount, maxPlays, this );
    vl->addWidget( plays );
    plays->setObjectName( "plays" );

    vl->addStretch();
}

