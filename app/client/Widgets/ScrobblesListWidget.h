/*
   Copyright 2010 Last.fm Ltd.
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

#ifndef SCROBBLES_LIST_WIDGET_H
#define SCROBBLES_LIST_WIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QPointer>

#include <lastfm/Track.h>

namespace lastfm { class Track; }
using lastfm::Track;

namespace unicorn { class Session; }

class QNetworkReply;

class ScrobblesListWidget : public QListWidget
{
    Q_OBJECT
public:
    ScrobblesListWidget( QWidget* parent = 0 );

signals:
    void trackClicked( class TrackWidget& );

public slots:
    void refresh();

private slots: 
    void onItemClicked( class TrackWidget& index );
    void onMoreClicked();

    void onTrackStarted( const lastfm::Track& track, const lastfm::Track& );

    void onSessionChanged( const unicorn::Session& session );

    void onResumed();
    void onPaused();
    void onStopped();

    void onGotRecentTracks();

    void onScrobblesSubmitted( const QList<lastfm::Track>& tracks );

    void onTrackWidgetRemoved();

    void write();
    void doWrite();

#ifdef Q_OS_MAC
    void scroll();
#endif

private:
    QString price( const QString& price, const QString& currency ) const;

    void read();

    QList<lastfm::Track> addTracks( const QList<lastfm::Track>& tracks );
    void limit( int limit );

    void hideScrobbledNowPlaying();

    void showEvent(QShowEvent *);

    void fetchTrackInfo( const QList<lastfm::Track>& tracks );

    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );

    void onRefreshing( bool refreshing );

private:
    QString m_path;

    QPointer<QTimer> m_writeTimer;
    QPointer<QNetworkReply> m_recentTrackReply;

    lastfm::Track m_track;
    class ScrobblesListWidgetItem* m_refreshItem;
    class ScrobblesListWidgetItem* m_trackItem;
    class ScrobblesListWidgetItem* m_moreItem;
};


#endif //ACTIVITY_LIST_WIDGET_H

