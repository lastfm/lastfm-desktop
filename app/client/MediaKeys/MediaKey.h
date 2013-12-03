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

#ifndef MEDIA_KEY_H
#define MEDIA_KEY_H

#include <QApplication>
#include <QObject>

#include <lastfm/Track.h>

namespace unicorn { class Session; }

class MediaKey : public QObject
{
    Q_OBJECT
public:
    explicit MediaKey( QObject* parent );

    void setEnabled( bool enabled );

    bool macEventFilter( EventHandlerCallRef, EventRef event );

private slots:
    void initialize();
    void applicationDidFinishLaunching( void* aNotification );
    void onSessionChanged( const unicorn::Session& session );
    void onTrackStarted( const lastfm::Track& newTrack, const lastfm::Track& oldTrack );

private:
    bool m_lastTrackRadio;
};

#endif // MEDIA_KEY_H
