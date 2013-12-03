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

#include <QDesktopServices>
#include <QCoreApplication>
#include <QUrl>

#include <lastfm/UrlBuilder.h>

#include "DesktopServices.h"

unicorn::DesktopServices::DesktopServices()
{
}


void
unicorn::DesktopServices::openUrl( QUrl url )
{
    if ( lastfm::UrlBuilder::isHost( url ) )
    {
        url.addQueryItem( "utm_source", "last.fm" );
        url.addQueryItem( "utm_medium", "application" );
        url.addQueryItem( "utm_campaign", "last.fm_desktop_application" );
        url.addQueryItem( "utm_content", QCoreApplication::applicationVersion() );
#ifdef WIN32
        url.addQueryItem( "utm_term", "WIN" );
#elif __APPLE__
        url.addQueryItem( "utm_term", "OSX" );
#elif defined (Q_WS_X11)
        url.addQueryItem( "utm_term", "X11" );
#endif
    }

    QDesktopServices::openUrl( url );
}
