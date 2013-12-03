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

#include <QPushButton>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QStackedLayout>

#include <lastfm/Track.h>
#include <lastfm/RadioStation.h>

#include "../Services/RadioService/RadioService.h"
#include "../Services/ScrobbleService/ScrobbleService.h"

#include "NowPlayingStackedWidget.h"
#include "NowPlayingWidget.h"
#include "NothingPlayingWidget.h"

NowPlayingStackedWidget::NowPlayingStackedWidget( QWidget* parent )
    :unicorn::SlidingStackedWidget( parent )
{
    addWidget( ui.nothingPlaying = new NothingPlayingWidget( this ) );
    addWidget( ui.nowPlaying = new NowPlayingWidget( this ) );

    connect( &RadioService::instance(), SIGNAL(tuningIn(RadioStation)), SLOT(showNowPlaying()) );
    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), SLOT(showNowPlaying()));
    connect( &ScrobbleService::instance(), SIGNAL(stopped()), SLOT(showNothingPlaying()));
}

void
NowPlayingStackedWidget::showNowPlaying()
{
    slide( 1 );
}

void
NowPlayingStackedWidget::showNothingPlaying()
{
    slide( 0 );
}


