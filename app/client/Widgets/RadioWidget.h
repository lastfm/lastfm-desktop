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

#ifndef RADIOWIDGET_H
#define RADIOWIDGET_H

#include <QPointer>
#include <QFrame>

namespace lastfm { class User; }
namespace lastfm { class RadioStation; }
namespace lastfm { class Track; }
using lastfm::RadioStation;
using lastfm::Track;

namespace unicorn { class Session; }

namespace Ui { class RadioWidget; }

class RadioWidget : public QFrame
{
    Q_OBJECT
public:
    explicit RadioWidget(QWidget *parent = 0);
    ~RadioWidget();

private slots:
    void onSessionChanged( const unicorn::Session& session );

    void onTuningIn( const RadioStation& station );
    void onRadioStopped();
    void onTrackStarted( const lastfm::Track& track , const lastfm::Track& oldTrack );

    void onGotRecentStations();

    void onSubscribeClicked();
    void onListenClicked();

private:
    void refresh( const unicorn::Session& session );

private:
    Ui::RadioWidget* ui;

    QString m_currentUsername;

    QPointer<QMovie> m_movie;
};

#endif // RADIOWIDGET_H
