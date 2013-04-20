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

#include <QShortcut>
#include <QMenu>
#include <QMovie>

#include "lib/unicorn/UnicornSettings.h"

#include "../Application.h"
#include "../Services/RadioService.h"
#include "../Services/ScrobbleService.h"
#include "../Services/AnalyticsService.h"

#include "PlaybackControlsWidget.h"
#include "ui_PlaybackControlsWidget.h"

PlaybackControlsWidget::PlaybackControlsWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PlaybackControlsWidget),
    m_scrobbleTrack( false )
{
    ui->setupUi(this);

    ui->volumeSlider->setAudioOutput( RadioService::instance().audioOutput() );

    ui->play->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->ban->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->love->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->skip->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->volume->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->volMax->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->volMin->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->volumeSlider->setAttribute( Qt::WA_LayoutUsesWidgetRect );

    ui->play->setAttribute( Qt::WA_MacNoClickThrough );
    ui->ban->setAttribute( Qt::WA_MacNoClickThrough );
    ui->love->setAttribute( Qt::WA_MacNoClickThrough );
    ui->skip->setAttribute( Qt::WA_MacNoClickThrough );

    // If the actions are triggered we should do something
    // love is dealt with by the application
    connect( aApp->banAction(), SIGNAL(triggered(bool)), SLOT(onBanClicked()) );
    connect( aApp->playAction(), SIGNAL(triggered(bool)), SLOT(onPlayClicked(bool)) );
    connect( aApp, SIGNAL(skipTriggered()), SLOT(onSkipClicked()) );

    m_playAction = new QAction( tr( "Play" ), aApp );
    connect( m_playAction, SIGNAL(triggered(bool)), aApp->playAction(), SLOT(trigger()) );
    connect( aApp->playAction(), SIGNAL(toggled(bool)), m_playAction, SLOT(setChecked(bool)) );

    // make sure this widget updates if the actions are changed elsewhere
    connect( aApp->loveAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );
    connect( aApp->banAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );
    connect( aApp->playAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );
    connect( aApp->skipAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );

    connect( &RadioService::instance(), SIGNAL(tuningIn(RadioStation)), SLOT(onTuningIn(RadioStation)));
    connect( &RadioService::instance(), SIGNAL(error(int,QVariant)), SLOT(onError(int, QVariant)));

    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(Track,Track)), SLOT(onTrackStarted(Track,Track)) );
    connect( &ScrobbleService::instance(), SIGNAL(stopped()), SLOT(onStopped()));
    connect( &ScrobbleService::instance(), SIGNAL(scrobblingOnChanged(bool)), SLOT(update()));

    onActionsChanged();

    // if our buttons are pressed we should trigger the actions
    connect( ui->love, SIGNAL(clicked()), aApp->loveAction(), SLOT(trigger()));
    connect( ui->ban, SIGNAL(clicked()), aApp->banAction(), SLOT(trigger()));
    connect( ui->play, SIGNAL(clicked()), aApp->playAction(), SLOT(trigger()));
    connect( ui->skip, SIGNAL(clicked()), aApp->skipAction(), SLOT(trigger()));

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );

    onSessionChanged( aApp->currentSession() );
}

void
PlaybackControlsWidget::onSessionChanged( const unicorn::Session& session )
{
    // don't change them until the session is valid
    if ( session.isValid() )
    {
        aApp->playAction()->setVisible( session.youRadio() );
        aApp->skipAction()->setVisible( session.youRadio() );
        aApp->banAction()->setVisible( session.youRadio() );
        m_playAction->setVisible( session.youRadio() );

        onActionsChanged();
    }
}

void
PlaybackControlsWidget::addToMenu( QMenu& menu, QAction* before )
{
    menu.insertAction( before, m_playAction );

    menu.insertSeparator( before );

    menu.insertAction( before, aApp->skipAction() );

    menu.insertSeparator( before );

    menu.insertAction( before, aApp->loveAction() );
    menu.insertAction( before, aApp->banAction() );

    menu.insertSeparator( before );

    menu.insertAction( before, aApp->tagAction() );
    menu.insertAction( before, aApp->shareAction() );

    menu.insertSeparator( before );

    //menu.addAction( tr( "Volume Up" ), &RadioService::instance(), SLOT(volumeUp()), QKeySequence( Qt::CTRL + Qt::Key_Up ));
    //menu.addAction( tr( "Volume Down" ), &RadioService::instance(), SLOT(volumeDown()), QKeySequence( Qt::CTRL + Qt::Key_Down ));
    menu.insertAction( before, aApp->muteAction() );
}


PlaybackControlsWidget::~PlaybackControlsWidget()
{
    delete ui;
}


void
PlaybackControlsWidget::setScrobbleTrack( bool scrobbleTrack )
{
    m_scrobbleTrack = scrobbleTrack;
    style()->polish( this );
    style()->polish( ui->details );
    style()->polish( ui->status );
    style()->polish( ui->device );
}

void
PlaybackControlsWidget::onActionsChanged()
{
   ui->love->setChecked( aApp->loveAction()->isChecked() );
   ui->ban->setChecked( aApp->banAction()->isChecked() );
   ui->play->setChecked( aApp->playAction()->isChecked() );
   ui->skip->setChecked( aApp->skipAction()->isChecked() );

   m_playAction->setText( aApp->playAction()->isChecked() ? tr( "Pause" ) : RadioService::instance().state() == Stopped ? tr( "Play" ) : tr( "Resume" ) );

   ui->love->setEnabled( aApp->loveAction()->isEnabled() );
   ui->ban->setEnabled( aApp->banAction()->isEnabled() && aApp->currentSession().youRadio()  );
   ui->play->setEnabled( aApp->playAction()->isEnabled() && aApp->currentSession().youRadio()  );
   ui->skip->setEnabled( aApp->skipAction()->isEnabled() && aApp->currentSession().youRadio()  );

   ui->love->setToolTip( ui->love->isChecked() ? tr("Unlove") : tr("Love") );
   ui->ban->setToolTip( tr("Ban") );
   ui->play->setToolTip( ui->play->isChecked() ? tr("Pause") : tr("Play") );
   ui->skip->setToolTip( tr("Skip") );

   ui->love->setText( ui->love->isChecked() ? tr("Unlove") : tr("Love") );
   ui->ban->setText( tr("Ban") );
   ui->play->setText( ui->play->isChecked() ? tr("Pause") : tr("Play") );
   ui->skip->setText( tr("Skip") );
}


void
PlaybackControlsWidget::onSpace()
{
    aApp->playAction()->trigger();
}


void
PlaybackControlsWidget::onPlayClicked( bool checked )
{
    if ( checked )
    {
        AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, PLAY_CLICKED, "PlayButtonPressed");

        if ( RadioService::instance().state() == Stopped )
            RadioService::instance().play( RadioStation( "" ) );
        else
        {
            RadioService::instance().resume();
        }
    }
    else
    {
        AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, PLAY_CLICKED, "PauseButtonPressed");
        RadioService::instance().pause();
    }
}

void
PlaybackControlsWidget::onSkipClicked()
{
    RadioService::instance().skip();
    AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, SKIP_CLICKED, "SkipClicked");
}


void
PlaybackControlsWidget::onLoveClicked( bool loved )
{
    MutableTrack track( RadioService::instance().currentTrack() );

    if ( loved )
    {
        track.love();
        AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, LOVE_TRACK, "TrackLoved");
    }
    else
    {
        track.unlove();
        AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, LOVE_TRACK, "TrackUnLoved");
    }

    connect( track.signalProxy(), SIGNAL(loveToggled(bool)), ui->love, SLOT(setChecked(bool)));
}


void
PlaybackControlsWidget::onLoveTriggered( bool loved )
{
    ui->love->setChecked( loved );
    onLoveClicked( loved );
}


void
PlaybackControlsWidget::onBanClicked()
{
    QNetworkReply* banReply = MutableTrack( RadioService::instance().currentTrack() ).ban();
    connect(banReply, SIGNAL(finished()), SLOT(onBanFinished()));

    AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, BAN_TRACK, "BanTrackPressed");
}


void
PlaybackControlsWidget::onBanFinished()
{
    lastfm::XmlQuery lfm;
    lfm.parse( static_cast<QNetworkReply*>(sender()) );

    if ( lfm.attribute( "status" ) != "ok" )
    {
    }

    aApp->skipAction()->trigger();
}

void
PlaybackControlsWidget::onTuningIn( const RadioStation& station )
{
    setScrobbleTrack( false );

    ui->icon->setPixmap( QPixmap( ":/control_bar_radio_as.png" ) );

    ui->status->setText( tr("Tuning") );
    ui->device->setText( station.title().isEmpty() ? tr( "A Radio Station" ) : station.title() );

    ui->play->setChecked( true );
    aApp->playAction()->setChecked( true );

    ui->play->setChecked( false );
    aApp->playAction()->setChecked( false );

    ui->progressBar->setTrack( Track() );

    aApp->playAction()->setEnabled( true );
    aApp->loveAction()->setEnabled( false );
    aApp->banAction()->setEnabled( false );
    aApp->skipAction()->setEnabled( false );
    aApp->tagAction()->setEnabled( false );
    aApp->shareAction()->setEnabled( false );

    ui->controls->show();
}

void
PlaybackControlsWidget::onTrackStarted( const Track& track, const Track& oldTrack )
{
    ui->progressBar->setTrack( track );

    disconnect( oldTrack.signalProxy(), SIGNAL(loveToggled(bool)), ui->love, SLOT(setChecked(bool)));

    if ( track != Track() )
    {
        disconnect( &RadioService::instance(), SIGNAL(tick(qint64)), this, SLOT(onTick(qint64)));
        disconnect( &ScrobbleService::instance(), SIGNAL(frameChanged(int)), ui->progressBar, SLOT(onFrameChanged(int)) );

        // you can love tag and share all tracks
        aApp->loveAction()->setEnabled( true );
        aApp->tagAction()->setEnabled( true );
        aApp->shareAction()->setEnabled( true );

        // play is always enabled as you should always
        // be able to start the radio
        aApp->playAction()->setEnabled( true );

        aApp->playAction()->setChecked( track.source() == Track::LastFmRadio );

        // can only ban and skip radio tracks
        aApp->banAction()->setEnabled( track.source() == Track::LastFmRadio );
        aApp->skipAction()->setEnabled( track.source() == Track::LastFmRadio );

        aApp->loveAction()->setChecked( track.isLoved() );

        ui->controls->setVisible( track.source() == Track::LastFmRadio );

        setScrobbleTrack( track.source() != Track::LastFmRadio  );

        if( track.source() == Track::LastFmRadio )
        {
            // A radio track!
            connect( track.signalProxy(), SIGNAL(loveToggled(bool)), ui->love, SLOT(setChecked(bool)));

            ui->status->setText( tr("Listening to") );
            QString title = RadioService::instance().station().title();
            ui->device->setText( title.isEmpty() ? tr( "A Radio Station" ) : title );

            connect( &RadioService::instance(), SIGNAL(tick(qint64)), SLOT(onTick(qint64)) );
        }
        else
        {
            // Not a radio track

            if ( track.extra( "playerId" ) == "spt" || track.extra( "playerId" ) == "mpris2" )
                ui->status->setText( tr("Listening to") );
            else
                ui->status->setText( tr("Scrobbling from") );

            ui->device->setText( track.extra( "playerName" ) );

            connect( &ScrobbleService::instance(), SIGNAL(frameChanged(int)), ui->progressBar, SLOT(onFrameChanged(int)) );
        }

        // Set the icon!
        QString id = track.extra( "playerId" );

        if ( id == "osx" || id == "itw" )
            ui->icon->setPixmap( QPixmap( ":/control_bar_scrobble_itunes.png" ) );
        else if (id == "foo")
            ui->icon->setPixmap( QPixmap( ":/control_bar_scrobble_foobar.png" ) );
        else if (id == "wa2")
            ui->icon->setPixmap( QPixmap( ":/control_bar_scrobble_winamp.png" ) );
        else if (id == "wmp")
            ui->icon->setPixmap( QPixmap( ":/control_bar_scrobble_wmp.png" ) );
        else if (id == "spt")
            ui->icon->setPixmap( QPixmap( ":/control_bar_scrobble_spotify.png" ) );
        else if ( id == "mpris2" )
        {
            if ( QIcon::hasThemeIcon( track.extra( "desktopEntry" ) ) )
                ui->icon->setPixmap( QIcon::fromTheme( track.extra( "desktopEntry" ) ).pixmap( 44, 44 ) );
            else if ( QIcon::hasThemeIcon( track.extra( "serviceName" ) ) )
                ui->icon->setPixmap( QIcon::fromTheme( track.extra( "serviceName" ) ).pixmap( 44, 44 ) );
            else if ( QIcon::hasThemeIcon( track.extra( "playerName" ) ) )
                ui->icon->setPixmap( QIcon::fromTheme( track.extra( "playerName" ) ).pixmap( 44, 44 ) );
            else
                ui->icon->setPixmap( QPixmap( ":/control_bar_radio_as.png" ) );
        }
        else
            ui->icon->setPixmap( QPixmap( ":/control_bar_radio_as.png" ) );
    }
}

void
PlaybackControlsWidget::onTick( qint64 tick )
{
    ui->progressBar->onFrameChanged( tick );
}

void
PlaybackControlsWidget::onError( int /*error*/, const QVariant& /*errorText*/ )
{
}

void
PlaybackControlsWidget::onStopped()
{
    ui->progressBar->setTrack( Track() );
    aApp->playAction()->setChecked( false );

    aApp->playAction()->setEnabled( true );
    aApp->loveAction()->setEnabled( false );
    aApp->banAction()->setEnabled( false );
    aApp->skipAction()->setEnabled( false );
    aApp->tagAction()->setEnabled( false );
    aApp->shareAction()->setEnabled( false );
}

