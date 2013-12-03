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

#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QMovie>

#include <lastfm/RadioStation.h>
#include <lastfm/XmlQuery.h>
#include <lastfm/Track.h>
#include <lastfm/UrlBuilder.h>

#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/DesktopServices.h"

#include "../Services/RadioService/RadioService.h"
#include "../Services/ScrobbleService/ScrobbleService.h"
#include "../Application.h"

#include "PlayableItemWidget.h"
#include "QuickStartWidget.h"

#include "ui_RadioWidget.h"
#include "RadioWidget.h"

#define MAX_RECENT_STATIONS 50

RadioWidget::RadioWidget(QWidget *parent)
    :QFrame( parent ), ui( new Ui::RadioWidget )
{
    ui->setupUi( this );

    ui->lastStationLabel->setObjectName( "title" );
    ui->personalLabel->setObjectName( "title" );
    ui->networkLabel->setObjectName( "title" );
    ui->recentLabel->setObjectName( "title" );

    ui->splitter->setObjectName( "splitter" );
    ui->splitter_2->setObjectName( "splitter" );
    ui->splitter_3->setObjectName( "splitter" );
    ui->splitter_4->setObjectName( "splitter" );

    connect( ui->subscribe, SIGNAL(clicked()), SLOT(onSubscribeClicked()) );
    connect( ui->listen, SIGNAL(clicked()), SLOT(onListenClicked()) );

    // need to know when we are playing the radio so we can switch between now playing and last playing
    connect( &RadioService::instance(), SIGNAL(tuningIn(RadioStation)), SLOT(onTuningIn(RadioStation) ) );
    connect( &RadioService::instance(), SIGNAL(stopped()), SLOT(onRadioStopped()));
    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), SLOT(onTrackStarted(lastfm::Track,lastfm::Track)) );

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session) ) );

    m_movie = new QMovie( ":/loading_meta.gif", "GIF", this );
    m_movie->setCacheMode( QMovie::CacheAll );
    ui->spinner->setMovie( m_movie );

    refresh( aApp->currentSession() );
}

RadioWidget::~RadioWidget()
{
    delete ui;
}

void
RadioWidget::onSessionChanged( const unicorn::Session& session )
{
    refresh( session );
}

void
RadioWidget::refresh( const unicorn::Session& session )
{
    if ( session.isValid() )
    {
        m_movie->stop();

        if ( session.youRadio() )
        {
            unicorn::UserSettings us( session.user().name() );
            QString stationUrl = us.value( "lastStationUrl", "" ).toString();
            QString stationTitle = us.value( "lastStationTitle", tr( "A Radio Station" ) ).toString();

            ui->nowPlayingFrame->setVisible( !stationUrl.isEmpty() );

            RadioStation lastStation( stationUrl );
            lastStation.setTitle( stationTitle );

            ui->lastStation->setStation( lastStation, stationTitle );
            ui->lastStation->setObjectName( "station" );
            style()->polish( ui->lastStation );

            ui->library->setStation( RadioStation::library( session.user() ), tr( "My Library Radio" ), tr( "Music you know and love" ) );
            ui->mix->setStation( RadioStation::mix( session.user() ), tr( "My Mix Radio" ), tr( "Your library plus new music" ) );
            ui->rec->setStation( RadioStation::recommendations( session.user() ), tr( "My Recommended Radio" ), tr( "New music from Last.fm" ) );

            ui->friends->setStation( RadioStation::friends( session.user() ), tr( "My Friends' Radio" ), tr( "Music your friends like" ) );
            ui->neighbours->setStation( RadioStation::neighbourhood( session.user() ), tr( "My Neighbourhood Radio" ), tr ( "Music from listeners like you" ) );

            if ( m_currentUsername != session.user().name() )
            {
                m_currentUsername = session.user().name();

                // clear the recent stations
                QLayout* recentStationsLayout = ui->recentStations->layout();
                while ( recentStationsLayout->count() )
                {
                    QLayoutItem* item = recentStationsLayout->takeAt( 0 );
                    delete item->widget();
                    delete item;
                }

                // fetch recent stations
                connect( session.user().getRecentStations( MAX_RECENT_STATIONS ), SIGNAL(finished()), SLOT(onGotRecentStations()));
            }

            ui->stackedWidget->setCurrentWidget( ui->mainPage );
        }
        else
        {
            ui->listen->setVisible( session.registeredWebRadio() );

            ui->stackedWidget->setCurrentWidget( ui->nonSubPage );

            ui->title->setText( tr( "Subscribe to listen to radio, only %1 a month" ).arg( session.subscriptionPriceString() ) );
            ui->subscribe->setVisible( session.subscriberRadio() );
        }
    }
    else
    {
        ui->stackedWidget->setCurrentWidget( ui->spinnerPage );
        m_movie->start();
    }
}

void
RadioWidget::onGotRecentStations()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {
        foreach ( const lastfm::XmlQuery& station, lfm["recentstations"].children("station") )
        {
            QString stationUrl = station["url"].text();

            if ( !stationUrl.startsWith( "lastfm://user/" + User().name() ) )
            {
                PlayableItemWidget* item = new PlayableItemWidget( RadioStation( stationUrl ), station["name"].text() );
                item->setObjectName( "station" );
                ui->recentStations->layout()->addWidget( item );
            }
        }
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}

void
RadioWidget::onTuningIn( const RadioStation& station )
{
    // Save this as the last station played
    ui->lastStationLabel->setText( tr( "Now Playing" ) );
    ui->lastStation->setStation( station.url(), station.title().isEmpty() ? tr( "A Radio Station" ) : station.title() );

    if ( !station.url().isEmpty() )
        ui->nowPlayingFrame->show();

    // insert at the front of the list

    if ( ui->recentStations && ui->recentStations->layout()
         && !station.url().isEmpty()
         && !station.url().startsWith( "lastfm://user/" + User().name() ) )
    {
        // if it exists already remove it
        for ( int i = 0 ; i < ui->recentStations->layout()->count() ; ++i )
        {
            if ( station.url() == qobject_cast<PlayableItemWidget*>(ui->recentStations->layout()->itemAt( i )->widget())->station().url() )
            {
                QLayoutItem* item = ui->recentStations->layout()->takeAt( i );
                item->widget()->deleteLater();
                delete item;
                break;
            }
        }

        // insert the new one at the beginning
        PlayableItemWidget* newItem = new PlayableItemWidget( station, station.title(), "", this );
        newItem->setObjectName( "station" );
        qobject_cast<QBoxLayout*>(ui->recentStations->layout())->insertWidget( 0, newItem );

        // limit the stations
        if ( ui->recentStations->layout()->count() > MAX_RECENT_STATIONS )
        {
            QLayoutItem* item = ui->recentStations->layout()->takeAt( ui->recentStations->layout()->count() - 1 );
            item->widget()->deleteLater();
            delete item;
        }
    }
}

void
RadioWidget::onRadioStopped()
{
    ui->lastStationLabel->setText( tr( "Last Station" ) );
}

void
RadioWidget::onTrackStarted( const lastfm::Track& track, const lastfm::Track& /*oldTrack*/ )
{
    // if a track starts and it's not a radio track, we are no longer listening to the radio
    if ( track.source() == Track::LastFmRadio )
    {
        ui->lastStationLabel->setText( tr( "Now Playing" ) );
    }
    else
    {
        ui->lastStationLabel->setText( tr( "Last Station" ) );
    }
}

void
RadioWidget::onSubscribeClicked()
{
    unicorn::DesktopServices::openUrl( UrlBuilder( "subscribe" ).url() );
}

void
RadioWidget::onListenClicked()
{
    unicorn::DesktopServices::openUrl( UrlBuilder( "listen" ).url() );
}
