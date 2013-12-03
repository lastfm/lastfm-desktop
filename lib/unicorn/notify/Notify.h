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

#ifndef NOTIFY_H
#define NOTIFY_H

#include <QObject>
#include <QPointer>

#include "lib/unicorn/TrackImageFetcher.h"

namespace lastfm { class Track; }

namespace unicorn
{

class Notify : public QObject
{
    Q_OBJECT
public:
    explicit Notify(QObject *parent = 0);
    ~Notify();

signals:
    void clicked();

public slots:
    void newTrack( const lastfm::Track& track );
    void paused();
    void resumed();
    void stopped();

private slots:
    void onFinished( const QPixmap& pixmap );

public:
    void growlNotificationWasClicked();

private:
    QPointer<TrackImageFetcher> m_trackImageFetcher;
};

}

#endif // NOTIFY_H
