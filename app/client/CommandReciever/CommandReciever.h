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

#ifndef COMMANDRECIEVER_H
#define COMMANDRECIEVER_H

#include <QObject>
#include <QPointer>
#include <QPixmap>

#include <lib/unicorn/TrackImageFetcher.h>
#include <lib/unicorn/UnicornApplicationDelegate.h>

class CommandReciever : public QObject, public unicorn::UnicornApplicationDelegateCommandObserver
{
    Q_OBJECT
public:
    explicit CommandReciever(QObject *parent = 0);
    ~CommandReciever();

    bool artworkDownloaded() const;
    QPixmap getArtwork() const;
    Track track() const;

private:
    QString trackTitle() const;
    QString artist() const;
    QString album() const;
    int duration();
    QPixmap artwork();
    bool loved();

private slots:
    void onFinished( const class QPixmap& image );

    void onTrackSpooled( const Track& track );
    void onStopped();

private:
    QPointer<TrackImageFetcher> m_trackImageFetcher;
    QPixmap m_pixmap;
    bool m_artworkDownloaded;
};

#endif // COMMANDRECIEVER_H
