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

#ifndef AUDIOSCROBBLER_SETTINGS_H
#define AUDIOSCROBBLER_SETTINGS_H

#include "lib/unicorn/UnicornSettings.h"

class AudioscrobblerSettings : public unicorn::AppSettings
{
public:
    AudioscrobblerSettings();

    bool raiseShortcutEnabled() const;
    Qt::KeyboardModifiers raiseShortcutModifiers() const;
    int raiseShortcutKey() const;
    QString raiseShortcutDescription() const;

    void setRaiseShortcutEnabled( bool f );
    void setRaiseShortcutKey( int key );
    void setRaiseShortcutModifiers( Qt::KeyboardModifiers m );
    void setRaiseShortcutDescription( QString d );
};

#endif // AUDIOSCROBBLER_SETTINGS_H
