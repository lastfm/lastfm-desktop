/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QMovie>

#include "NowPlayingWidget.h"
#include "PlaybackControlsWidget.h"
#include "MetadataWidget.h"

#include "../Services/RadioService.h"
#include "../Services/ScrobbleService.h"

NowPlayingWidget::NowPlayingWidget(QWidget *parent)
    :QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );

    layout->addWidget( ui.playbackControls = new PlaybackControlsWidget( this ) );
    layout->addWidget( ui.stack = new QStackedWidget( this ) );

    ui.stack->addWidget( ui.spinner = new QLabel() );
    ui.spinner->setObjectName( "spinner" );
    ui.spinner->setAlignment( Qt::AlignCenter );

    m_movie = new QMovie( ":/loading_meta.gif", "GIF", this );
    m_movie->setCacheMode( QMovie::CacheAll );
    ui.spinner->setMovie ( m_movie );

    ui.metadata = 0;

    connect( &RadioService::instance(), SIGNAL(tuningIn(RadioStation)), SLOT(onTuningIn(RadioStation)) );

    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), SLOT(onTrackStarted(lastfm::Track,lastfm::Track)) );
    connect( &ScrobbleService::instance(), SIGNAL(stopped()), SLOT(onStopped()) );
}

PlaybackControlsWidget*
NowPlayingWidget::playbackControls() const
{
    return ui.playbackControls;
}

void
NowPlayingWidget::onTuningIn( const RadioStation& )
{   
    ui.stack->setCurrentWidget( ui.spinner );
    m_movie->start();
}

void
NowPlayingWidget::onTrackStarted( const lastfm::Track& track, const lastfm::Track& )
{
    if ( track != Track() )
    {
        setUpdatesEnabled( false );

        if ( ui.metadata )
        {
            ui.stack->removeWidget( ui.metadata );
            ui.metadata->deleteLater();
        }

        ui.stack->addWidget( ui.metadata = new MetadataWidget( track, this ) );
        ui.metadata->setBackButtonVisible( false );

        ui.stack->setCurrentWidget( ui.metadata );
        m_movie->stop();

        setUpdatesEnabled( true );
    }
}

void
NowPlayingWidget::onStopped()
{
    ui.stack->setCurrentWidget( ui.spinner );
    m_movie->stop();
}
