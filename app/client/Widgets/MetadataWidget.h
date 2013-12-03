/*
   Copyright 2005-2009 Last.fm Ltd.
      - Primarily authored by Jono Cole and Doug Mansell

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

#ifndef METADATAWIDGET_H
#define METADATAWIDGET_H

#include <QWidget>
#include <QPointer>

#include <lastfm/Album.h>
#include <lastfm/Track.h>

#include "lib/unicorn/widgets/HttpImageWidget.h"

namespace Ui { class MetadataWidget; }

class DataListWidget;
class HttpImageWidget;
class QLabel;
class QGroupBox;
class BioWidget;
class QMovie;


class MetadataWidget : public QFrame
{
    Q_OBJECT
public:
    MetadataWidget( const Track& track, QWidget* p = 0 );
    ~MetadataWidget();

    void fetchTrackInfo( bool force );

    class ScrobbleControls* scrobbleControls() const;

    QWidget* basicInfoWidget();
    void setBackButtonVisible( bool );

    static QString getContextString( const Track& track );

private slots:
    void onTrackGotInfo(const QByteArray& data);
    void onAlbumGotInfo();
    void onArtistGotInfo();
    void onArtistGotEvents();
    void onTrackGotBuyLinks();
    void onBuyActionTriggered( QAction* buyAction );

    void onTrackGotYourTags();
    void onArtistGotYourTags();

    void listItemClicked( const class QModelIndex& );

    void onScrobblesCached( const QList<lastfm::Track>& tracks );
    void onScrobbleStatusChanged( short scrobbleStatus );

    void checkFinished();

signals:
    void lovedStateChanged(bool loved);
    void backClicked();
    void finished();

private:
    void setTrackDetails( const Track& track );

    QString contextString( const Track& track );
    QString scrobbleString( const Track& track );

    void showEvent( QShowEvent *e );

private:
    Ui::MetadataWidget *ui;

    Track m_track;

    int m_globalTrackScrobbles;
    int m_userTrackScrobbles;
    int m_globalArtistScrobbles;
    int m_userArtistScrobbles;
    int m_artistListeners;

    Album m_albumGuess;

    int m_numCalls;
    bool m_fetchedTrackInfo;

    QPointer<QMovie> m_movie;
};

#endif // METADATAWIDGET_H
