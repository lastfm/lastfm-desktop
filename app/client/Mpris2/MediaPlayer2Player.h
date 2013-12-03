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

#ifndef MEDIAPLAYER2PLAYER_H
#define MEDIAPLAYER2PLAYER_H

#include "DBusAbstractAdaptor.h"
#include "lib/listener/State.h"

#include <QDBusObjectPath>
#include <lastfm/Track.h>

class MediaPlayer2Player : public DBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player");

    Q_PROPERTY( QString PlaybackStatus READ PlaybackStatus )
    Q_PROPERTY( double Rate READ Rate WRITE SetRate )
    Q_PROPERTY( QVariantMap Metadata READ Metadata )
    Q_PROPERTY( double Volume READ Volume WRITE SetVolume )
    Q_PROPERTY( qlonglong Position READ Position )
    Q_PROPERTY( double MinimumRate READ MinimumRate )
    Q_PROPERTY( double MaximumRate READ MaximumRate )
    Q_PROPERTY( bool CanGoNext READ CanGoNext )
    Q_PROPERTY( bool CanGoPrevious READ CanGoPrevious )
    Q_PROPERTY( bool CanPlay READ CanPlay )
    Q_PROPERTY( bool CanPause READ CanPause )
    Q_PROPERTY( bool CanSeek READ CanSeek )
    Q_PROPERTY( bool CanControl READ CanControl )

public:
    MediaPlayer2Player( QObject* parent = 0 );

    QString PlaybackStatus() const;
    double Rate() const;
    void SetRate(double value);
    QVariantMap Metadata() const;
    double Volume() const;
    void SetVolume(double value);
    qlonglong Position() const;
    double MinimumRate() const;
    double MaximumRate() const;
    bool CanGoNext() const;
    bool CanGoPrevious() const;
    bool CanPlay() const;
    bool CanPause() const;
    bool CanSeek() const;
    bool CanControl() const;

public slots:
    void Next() const;
    void Previous() const;
    void Pause() const;
    void PlayPause() const;
    void Stop() const;
    void Play() const;
    void Seek( qlonglong Offset ) const;
    void SetPosition( const QDBusObjectPath& TrackId, qlonglong Position ) const;
    void OpenUri( QString Uri ) const;

signals:
    void Seeked( qlonglong Position ) const;

private slots:
    void onTrackChanged( const Track& track );
    void onVolumeChanged( qreal vol );
    void onPlaybackStateChanged();
    void onSessionInfo();

private:
    Track m_track;
    State m_playbackState;
};

#endif
