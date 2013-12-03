/*
   Copyright 2005-2011 Last.fm Ltd.
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

#include "AudioscrobblerSettings.h"

AudioscrobblerSettings::AudioscrobblerSettings()
{
}

Qt::KeyboardModifiers
AudioscrobblerSettings::raiseShortcutModifiers() const
{
    return (Qt::KeyboardModifiers)value( "raiseShortcutModifiers", (int)(Qt::ControlModifier | Qt::MetaModifier) ).toInt();
}

int
AudioscrobblerSettings::raiseShortcutKey() const
{
#ifdef Q_WS_MAC
    const int sKeyCode = 1;
    return value( "raiseShortcutKey", sKeyCode ).toInt();
#elif defined Q_WS_WIN
    const int sKeyCode = 83;
    return value( "raiseShortcutKey", sKeyCode ).toInt();
#endif

}

QString
AudioscrobblerSettings::raiseShortcutDescription() const
{
    return value( "raiseShortcutDescription", QString::fromUtf8(
#ifdef Q_OS_MAC
    "⌃⌘ S"
#else
    "Ctrl+Shift+S"
#endif
     ) ).toString();
}

void
AudioscrobblerSettings::setRaiseShortcutKey( int key )
{
    setValue( "raiseShortcutKey", key );
}

void
AudioscrobblerSettings::setRaiseShortcutModifiers( Qt::KeyboardModifiers m )
{
    setValue( "raiseShortcutModifiers", (int)m );
}

void
AudioscrobblerSettings::setRaiseShortcutDescription( QString d )
{
    setValue( "raiseShortcutDescription", d );
}
