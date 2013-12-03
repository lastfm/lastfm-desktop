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

#ifndef NOWPLAYINGSTACKEDWIDGET_H
#define NOWPLAYINGSTACKEDWIDGET_H

#include <QWidget>
#include <lastfm/Track.h>
#include <lastfm/RadioStation.h>

#include "lib/unicorn/widgets/SlidingStackedWidget.h"

class TrackItem;
class RadioProgressBar;
class QLabel;
class QImage;

class NowPlayingStackedWidget : public unicorn::SlidingStackedWidget
{
    Q_OBJECT

private:
    struct {
        class NothingPlayingWidget* nothingPlaying;
        class NowPlayingWidget* nowPlaying;
    } ui;

public:
    NowPlayingStackedWidget( QWidget* parent = 0 );

    class NowPlayingWidget* nowPlaying() const { return ui.nowPlaying; }

private slots:
    void showNowPlaying();
    void showNothingPlaying();
};

#endif // NOWPLAYINGSTACKEDWIDGET_H
