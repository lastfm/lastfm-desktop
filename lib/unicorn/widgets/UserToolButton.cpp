/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Jono Cole and Doug Mansell

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

#include "UserToolButton.h"
#include <QApplication>

#include <QDebug>
#include <QToolButton>
#include <QPainter>
#include <lastfm/User.h>

#include "UserMenu.h"

#include "../UnicornSettings.h"

using namespace lastfm;

UserToolButton::UserToolButton()
{
    setMouseTracking( true );
    setIconSize( QSize( 40, 40 ));
    setCheckable( true );

    if( unicorn::Settings().userRoster().count() > 1 ) {
        setMenu( new UserMenu());
        setPopupMode( QToolButton::MenuButtonPopup );
    }

    connect( this, SIGNAL( toggled( bool )), window(), SLOT( toggleProfile( bool )));
    connect( qApp, SIGNAL( sessionChanged(unicorn::Session)), SLOT( onSessionChanged(unicorn::Session)));
    connect( qApp, SIGNAL( gotUserInfo( lastfm::User )), SLOT( onUserGotInfo( lastfm::User )));
    connect( qApp, SIGNAL( rosterUpdated()), SLOT( onRosterUpdated()));
}

void 
UserToolButton::onSessionChanged( const unicorn::Session& /*session*/ )
{
    setIcon( QIcon() );
}

void 
UserToolButton::onUserGotInfo( const User& user )
{
    connect( lastfm::nam()->get(QNetworkRequest( user.imageUrl( lastfm::User::MediumImage))), SIGNAL( finished()),
                                                                          SLOT( onImageDownloaded()));
}

void 
UserToolButton::onImageDownloaded()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
    Q_ASSERT( reply );

    reply->deleteLater();

    QPixmap pm;
    if( !pm.loadFromData( reply->readAll()) )
        pm = QPixmap(":lastfm/default_user_small.png");

    QPixmap on( ":lastfm/profile_on.png" );
    QPixmap off( ":lastfm/profile_off.png" );
    QPixmap hover( ":lastfm/profile_hover.png" );

    QIcon icon;

    {
        QPainter p( &on );
        p.setRenderHint( QPainter::SmoothPixmapTransform );
        p.setCompositionMode( QPainter::CompositionMode_DestinationOver );
        p.drawPixmap( on.rect().adjusted( 5, 5, -5, -5 ), pm, pm.rect());
    }
    icon.addPixmap( on, QIcon::Normal, QIcon::On );

    {
        QPainter p( &off );
        p.setRenderHint( QPainter::SmoothPixmapTransform );
        p.setCompositionMode( QPainter::CompositionMode_DestinationOver );
        p.drawPixmap( off.rect().adjusted( 5, 5, -5, -5 ), pm, pm.rect());
    }
    icon.addPixmap( off, QIcon::Normal, QIcon::Off );

     {
        QPainter p( &hover );
        p.setRenderHint( QPainter::SmoothPixmapTransform );
        p.setCompositionMode( QPainter::CompositionMode_DestinationOver );
        p.drawPixmap( hover.rect().adjusted( 5, 5, -5, -5 ), pm, pm.rect());
    }
    icon.addPixmap( hover, QIcon::Active );

    setIcon( icon );
}

void 
UserToolButton::onRosterUpdated()
{
    if( unicorn::Settings().userRoster().count() > 1 ) {
        setMenu( new UserMenu());
        setPopupMode( QToolButton::MenuButtonPopup );
        setStyle( QApplication::style());
    } else {
        setPopupMode( QToolButton::DelayedPopup );
        menu()->deleteLater();
        setStyle( QApplication::style());
    }
}
