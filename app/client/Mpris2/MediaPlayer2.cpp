/*
   Copyright (C) 2013 John Stamp <jstamp@mehercule.net>

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

#include "MediaPlayer2.h"

#include "Application.h"

MediaPlayer2::MediaPlayer2( QObject* parent ) : DBusAbstractAdaptor( parent )
{
}


bool
MediaPlayer2::CanQuit() const
{
    return true;
}


bool
MediaPlayer2::CanRaise() const
{
    return true;
}


void
MediaPlayer2::Raise()
{
    aApp->showWindow();
}


void
MediaPlayer2::Quit()
{
    aApp->quit();
}


bool
MediaPlayer2::HasTrackList() const
{
    return false;
}


QString
MediaPlayer2::Identity() const
{
    return aApp->applicationName();
}


QString
MediaPlayer2::DesktopEntry() const
{
    return "lastfm-scrobbler";
}


QStringList
MediaPlayer2::SupportedUriSchemes() const
{
    return QStringList() << "lastfm";
}


QStringList
MediaPlayer2::SupportedMimeTypes() const
{
    return QStringList();
}
