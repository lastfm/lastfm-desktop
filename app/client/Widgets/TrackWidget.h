/*
   Copyright 2012 Last.fm Ltd.
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

#ifndef TRACKWIDGET_H
#define TRACKWIDGET_H

#include <QPushButton>

#include <lastfm/Track.h>

namespace Ui { class TrackWidget; }

class QMovie;
class TrackImageFetcher;

class TrackWidget : public QPushButton
{
    Q_OBJECT
    
public:
    explicit TrackWidget( Track& track, QWidget *parent = 0 );
    ~TrackWidget();

    void update( const lastfm::Track& track );

    void setTrack( lastfm::Track& track );
    lastfm::Track track() const;

    void setNowPlaying( bool nowPlaying );

public slots:
    void startSpinner();
    void clearSpinner();

signals:
    void clicked( TrackWidget& trackWidget );
    void removed();

private slots:
    void onClicked();
    void onLoveClicked( bool loved );
    void onTagClicked();
    void onShareClicked();

    void onBuyClicked();
    void onGotBuyLinks();

    void onRemoveClicked();
    void onRemovedScrobble();

    void onBuyActionTriggered( QAction* buyAction );

    void onShareLastFm();
    void onShareTwitter();
    void onShareFacebook();

    // signals from the track
    void onLoveToggled( bool loved );
    void onScrobbleStatusChanged();
    void onCorrected( QString correction );

    void play();
    void playNext();

    void updateTimestamp();

public:
    QSize sizeHint() const;

private:
    void setTrackDetails();
    void setTrackTitleWidth();

    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void contextMenuEvent( QContextMenuEvent* event );

    void fetchAlbumArt();

private:
    Ui::TrackWidget* ui;

    lastfm::Track m_track;

    QPointer<QMovie> m_movie;
    QPointer<QTimer> m_timestampTimer;
    QPointer<TrackImageFetcher> m_trackImageFetcher;

    bool m_nowPlaying;
    bool m_triedFetchAlbumArt;

    QPointer<QMovie> m_spinnerMovie;
    class QLabel* m_spinner;
};

#endif // TRACKWIDGET_H
