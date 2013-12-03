/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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
#ifndef ITUNES_SCRIPT_H
#define ITUNES_SCRIPT_H

#include <QThread>
#include <CoreFoundation/CoreFoundation.h>

#include "lib/unicorn/mac/AppleScript.h"

/** @author Max Howell <max@last.fm> */
class ITunesListener : public QObject
{
    Q_OBJECT
public:
    ITunesListener( QObject* parent );
    ~ITunesListener();

    enum State { Unknown = -1, Playing, Paused, Stopped };

signals:
    void newConnection( class PlayerConnection* );
    
private:
    static bool iTunesIsPlaying();

    /** iTunes notification center callback */
    static void callback( CFNotificationCenterRef, 
                          void*, 
                          CFStringRef, 
                          const void*, 
                          CFDictionaryRef );

    void callback( CFDictionaryRef );

private slots:
    void setupCurrentTrack();

private:
    State m_state;
    QString m_previousPid;
    struct ITunesConnection* m_connection;

    AppleScript m_currentTrackScript;
};

#endif
