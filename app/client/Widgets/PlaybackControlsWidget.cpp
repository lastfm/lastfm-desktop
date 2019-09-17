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

#include "../Application.h"
#include "../Services/ScrobbleService.h"
#include "../Services/AnalyticsService.h"

#include "PlaybackControlsWidget.h"
#include "ui_PlaybackControlsWidget.h"

#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/widgets/Label.h"

#include <QShortcut>
#include <QMenu>
#include <QMovie>
#include <QTimer>


void
setLayoutUsesWidgetRect( QWidget* widget )
{
    foreach ( QObject* object, widget->children() )
    {
        if ( object->isWidgetType() )
        {
            QWidget* widget = qobject_cast<QWidget*>( object );
            widget->setAttribute( Qt::WA_LayoutUsesWidgetRect );
            setLayoutUsesWidgetRect( widget );
        }
    }
}


PlaybackControlsWidget::PlaybackControlsWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PlaybackControlsWidget),
    m_scrobbleTrack( false )
{
    ui->setupUi(this);

    setLayoutUsesWidgetRect( this );

    ui->love->setAttribute( Qt::WA_MacNoClickThrough );

    // make sure this widget updates if the actions are changed elsewhere
    connect( aApp->loveAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );

    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), SLOT(onTrackStarted(lastfm::Track,lastfm::Track)) );
    connect( &ScrobbleService::instance(), SIGNAL(stopped()), SLOT(onStopped()));
    connect( &ScrobbleService::instance(), SIGNAL(scrobblingOnChanged(bool)), SLOT(update()));

    onActionsChanged();


    // if our buttons are pressed we should trigger the actions
    connect( ui->love, SIGNAL(clicked()), aApp->loveAction(), SLOT(trigger()));

    connect( &ScrobbleService::instance(), SIGNAL(frameChanged(int)), SLOT(onFrameChanged(int)) );
}

void
PlaybackControlsWidget::addToMenu( QMenu& menu, QAction* before )
{
    menu.insertAction( before, m_playAction );

    menu.insertSeparator( before );

    menu.insertSeparator( before );

    menu.insertAction( before, aApp->loveAction() );

    menu.insertSeparator( before );

    menu.insertAction( before, aApp->tagAction() );
    menu.insertAction( before, aApp->shareAction() );

    menu.insertSeparator( before );

    //menu.addAction( tr( "Volume Up" ), &RadioService::instance(), SLOT(volumeUp()), QKeySequence( Qt::CTRL + Qt::Key_Up ));
    //menu.addAction( tr( "Volume Down" ), &RadioService::instance(), SLOT(volumeDown()), QKeySequence( Qt::CTRL + Qt::Key_Down ));
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
    style()->polish( ui->love );
}

void
PlaybackControlsWidget::onActionsChanged()
{
   ui->love->setChecked( aApp->loveAction()->isChecked() );

   ui->love->setEnabled( aApp->loveAction()->isEnabled() );

   ui->love->setToolTip( ui->love->isChecked() ? tr("Unlove") : tr("Love") );

   ui->love->setText( ui->love->isChecked() ? tr("Unlove") : tr("Love") );
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
    }
    else
    {
        AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, PLAY_CLICKED, "PauseButtonPressed");
    }
}

void
PlaybackControlsWidget::onLoveClicked( bool loved )
{
    if ( loved )
    {
        AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, LOVE_TRACK, "TrackLoved");
    }
    else
    {
        AnalyticsService::instance().sendEvent(NOW_PLAYING_CATEGORY, LOVE_TRACK, "TrackUnLoved");
    }
}


void
PlaybackControlsWidget::onLoveTriggered( bool loved )
{
    ui->love->setChecked( loved );
    onLoveClicked( loved );
}


void
PlaybackControlsWidget::onTrackStarted( const lastfm::Track& track, const lastfm::Track& /*oldTrack*/ )
{
    setTrack( track );
}

void
PlaybackControlsWidget::onError( int /*error*/, const QVariant& /*errorText*/ )
{
}

void
PlaybackControlsWidget::onStopped()
{
    aApp->playAction()->setChecked( false );
    aApp->playAction()->setEnabled( true );
    aApp->loveAction()->setEnabled( false );
    aApp->tagAction()->setEnabled( false );
    aApp->shareAction()->setEnabled( false );
}

void
PlaybackControlsWidget::setTrack( const Track& track )
{
    // we're about to change loads of stuff to don't update until the end
    setUpdatesEnabled( false );

    disconnect( m_track.signalProxy(), SIGNAL(loveToggled(bool)), ui->love, SLOT(setChecked(bool)));
    disconnect( m_track.signalProxy(), SIGNAL(scrobbleStatusChanged(short)), this, SLOT(onScrobbleStatusChanged(short)) );
    m_track = track;
    connect( m_track.signalProxy(), SIGNAL(scrobbleStatusChanged(short)), this, SLOT(onScrobbleStatusChanged(short)) );

    ui->as->setPixmap( QPixmap( ":/scrobble_marker_OFF.png" ) );
    ui->as->setToolTip( "" );

    if ( track != Track() )
    {
        // you can love tag and share all tracks
        aApp->loveAction()->setEnabled( true );
        aApp->tagAction()->setEnabled( true );
        aApp->shareAction()->setEnabled( true );

        // play is always enabled as you should always
        // be able to start the radio
        aApp->playAction()->setEnabled( true );

        aApp->playAction()->setChecked( false );

        aApp->loveAction()->setChecked( track.isLoved() );

        bool externalPlayer = track.extra( "playerId" ) == "spt" || track.extra( "playerId" ) == "mpris2";
        bool scrobbleTrack = !externalPlayer;

        ui->love->setVisible( true ); // you can always love a track

        ui->as->setVisible( scrobbleTrack );
        ui->as->setToolTip( tr( "Not scrobbled" ) );

        ui->scrobbleMeter->setVisible( scrobbleTrack );
        ui->scrobbleMeter->setRange( 0, ScrobbleService::instance().stopWatch()->scrobblePoint() * 1000 );
        ui->scrobbleMeter->setValue( 0 );
        ui->scrobbleMeter->setToolTip( tr( "Scrobble meter: %1%" ).arg( 0 ) );

        ui->progressSpacer->setVisible( true );

        connect( track.signalProxy(), SIGNAL(loveToggled(bool)), ui->love, SLOT(setChecked(bool)));

        ui->status->setText( externalPlayer ? tr("Listening to") : tr("Scrobbling from") );

        setScrobbleTrack( true );

        ui->message->setVisible( externalPlayer );
        ui->message->setText( track.extra( "playerId" ) == "spt" ?
                                  unicorn::Label::boldLinkStyle( tr( "Enable scrobbling by getting the %1." ).arg( unicorn::Label::anchor( "https://www.last.fm/settings/applications", tr( "Last.fm app for Spotify" ) ) ), Qt::black ):
                                  "" );

        ui->device->setText( track.extra( "playerName" ) );

        // Set the icon!
        QString id = track.extra( "playerId" );

        if ( id == "osx" || id == "itw" )
            ui->icon->setPixmap( QPixmap( ":/control_bar_scrobble_itunes.png" ) );
        else if (id == "mac")
            ui->icon->setPixmap( QPixmap( ":/control_bar_scrobble_applemusic.png" ) );
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
    else
    {
        // what do we do with a null track???
    }

    setUpdatesEnabled( true );
}

bool
PlaybackControlsWidget::eventFilter( QObject *obj, QEvent *event )
{
    return QFrame::eventFilter( obj, event );
}

void
PlaybackControlsWidget::onFrameChanged( int frame )
{
    if ( ui->scrobbleMeter->maximum() != 1 )
    {
        int scrobbleValue = frame >= ui->scrobbleMeter->maximum() ? ui->scrobbleMeter->maximum() : frame;
        ui->scrobbleMeter->setRange( 0, ScrobbleService::instance().stopWatch()->scrobblePoint() * 1000 );
        ui->scrobbleMeter->setValue( scrobbleValue );
        ui->scrobbleMeter->setToolTip( tr( "Scrobble meter: %1%" ).arg( qRound( ( 100 * scrobbleValue ) / ui->scrobbleMeter->maximum() ) )  );
    }
}


void
PlaybackControlsWidget::onScrobbleStatusChanged( short scrobbleStatus )
{
    if ( scrobbleStatus != Track::Null )
        ui->as->setPixmap( QPixmap( ":/scrobble_marker_ON.png" ) );

    if ( scrobbleStatus == Track::Null ) ui->as->setToolTip( tr( "Not scrobbled" ) );
    else if ( scrobbleStatus == Track::Cached || scrobbleStatus == Track::Submitted ) ui->as->setToolTip( tr( "Scrobbled" ) );
    else if ( scrobbleStatus == Track::Error ) ui->as->setToolTip( tr( "Error: \"%1\"" ).arg( m_track.scrobbleErrorText() ) );
}
