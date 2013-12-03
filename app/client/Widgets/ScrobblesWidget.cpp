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

#include <QWidget>
#include <QLayoutItem>
#include <QDebug>

#include <lastfm/Track.h>

#include "../Services/AnalyticsService/AnalyticsService.h"
#include "ScrobblesListWidget.h"
#include "TrackWidget.h"
#include "MetadataWidget.h"
#include "ScrobblesWidget.h"
#include "ui_ScrobblesWidget.h"

ScrobblesWidget::ScrobblesWidget( QWidget* parent )
    :QWidget( parent ), ui( new Ui::ScrobblesWidget ), m_lastIndex( 0 )
{
    ui->setupUi( this );

    connect( ui->scrobbles, SIGNAL( trackClicked(TrackWidget&)), SLOT( onTrackClicked(TrackWidget&)));
    connect( ui->slidingWidget, SIGNAL( animationFinished()), SLOT(onMoveFinished()));

    ui->stackedWidget->setCurrentWidget( ui->scrobbles );
}

ScrobblesWidget::~ScrobblesWidget()
{
    delete ui;
}

void
ScrobblesWidget::refresh()
{
    ui->scrobbles->refresh();
}

void
ScrobblesWidget::onCurrentChanged( int index )
{
    if ( index == 1 && (m_lastIndex != index || ui->slidingWidget->currentWidget() != ui->stackedWidget ) )
    {
        // We've switch to this tab OR are moving back from Scrobble to ScrobbleList
        ui->scrobbles->refresh();
        AnalyticsService::instance().sendPageView( "Scrobbles" );
    }

    m_lastIndex = index;

    ui->slidingWidget->slide( ui->slidingWidget->indexOf( ui->stackedWidget ) );
}

void
ScrobblesWidget::onTrackClicked( TrackWidget& trackWidget )
{
    MetadataWidget* w;
    ui->slidingWidget->addWidget( w = new MetadataWidget( trackWidget.track() ));
    w->fetchTrackInfo( true );
    w->setBackButtonVisible( true );

    trackWidget.startSpinner();
    connect( ui->slidingWidget, SIGNAL( animationFinished()), &trackWidget, SLOT(clearSpinner()) );

    connect( w, SIGNAL(finished()), SLOT(onMetadataWidgetFinished()));
    connect( w, SIGNAL(backClicked()), SLOT(onBackClicked()));
}

void
ScrobblesWidget::onMetadataWidgetFinished()
{
    ui->slidingWidget->slide( ui->slidingWidget->currentIndex() + 1 );
    AnalyticsService::instance().sendPageView( "Scrobbles/Scrobble" );
}

void
ScrobblesWidget::onBackClicked()
{
    ui->slidingWidget->slide( ui->slidingWidget->indexOf( ui->stackedWidget ) );
    AnalyticsService::instance().sendPageView( "Scrobbles" );
}

void
ScrobblesWidget::onMoveFinished()
{
    if( ui->slidingWidget->currentWidget() == ui->stackedWidget )
    {
        while ( ui->slidingWidget->count() > 1 )
        {
            QWidget* widget = ui->slidingWidget->widget( 1 );
            ui->slidingWidget->removeWidget( widget );
            delete widget;
        }
    }
}
