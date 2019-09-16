/*
   Copyright 2010-2013 Last.fm Ltd.
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

#include <QStringList>

#include "AppleMusicPluginInfo.h"

#ifdef Q_OS_MAC
// the iTunes listener has a static method we can use to tell if iTunes is installed
#include "lib/listener/mac/ITunesListener.h"
#endif

unicorn::AppleMusicPluginInfo::AppleMusicPluginInfo( QObject* parent )
    :IPluginInfo( parent )
{}

QString
unicorn::AppleMusicPluginInfo::name() const
{
    return "Music";
}

QString
unicorn::AppleMusicPluginInfo::displayName() const
{
    return QString( "Music" );
}

QString
unicorn::AppleMusicPluginInfo::id() const
{
    return "mac";
}

bool
unicorn::AppleMusicPluginInfo::isAppInstalled() const
{
    return ITunesListener::getPlayerAppId().compare("com.music.Music") == 0;
}


