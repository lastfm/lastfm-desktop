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
#include "../Services/RadioService.h"
#include "../Services/ScrobbleService.h"
#include "../Services/AnalyticsService.h"

#include "PlaybackControlsWidget.h"
#include "VolumeSlider.h"
#include "ui_PlaybackControlsWidget.h"

#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/widgets/Label.h"

#include <phonon/MediaObject>
#include <phonon/SeekSlider>
#include <phonon/VolumeSlider>

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

    ui->play->setAttribute( Qt::WA_MacNoClickThrough );
    ui->ban->setAttribute( Qt::WA_MacNoClickThrough );
    ui->love->setAttribute( Qt::WA_MacNoClickThrough );
    ui->skip->setAttribute( Qt::WA_MacNoClickThrough );
    ui->volume->setAttribute( Qt::WA_MacNoClickThrough );

    // If the actions are triggered we should do something
    // love is dealt with by the application
    connect( aApp->banAction(), SIGNAL(triggered(bool)), SLOT(onBanClicked()) );
    connect( aApp->playAction(), SIGNAL(triggered(bool)), SLOT(onPlayClicked(bool)) );
    connect( aApp->muteAction(), SIGNAL(triggered(bool)), SLOT(mute()) );
    connect( aApp, SIGNAL(skipTriggered()), SLOT(onSkipClicked()) );

    m_playAction = new QAction( tr( "Play" ), aApp );
    connect( m_playAction, SIGNAL(triggered(bool)), aApp->playAction(), SLOT(trigger()) );
    connect( aApp->playAction(), SIGNAL(toggled(bool)), m_playAction, SLOT(setChecked(bool)) );

    // make sure this widget updates if the actions are changed elsewhere
    connect( aApp->loveAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );
    connect( aApp->banAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );
    connect( aApp->playAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );
    connect( aApp->skipAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );
    connect( aApp->skipAction(), SIGNAL(changed()), SLOT(onActionsChanged()) );

    connect( &RadioService::instance(), SIGNAL(tuningIn(RadioStation)), SLOT(onTuningIn(RadioStation)));
    connect( &RadioService::instance(), SIGNAL(error(int,QVariant)), SLOT(onError(int, QVariant)));

    connect( RadioService::instance().audioOutput(), SIGNAL(volumeChanged(qreal)), SLOT(onVolumeChanged(qreal)) );
    onVolumeChanged( RadioService::instance().audioOutput()->volume() );

    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), SLOT(onTrackStarted(lastfm::Track,lastfm::Track)) );
    connect( &ScrobbleService::instance(), SIGNAL(stopped()), SLOT(onStopped()));
    connect( &ScrobbleService::instance(), SIGNAL(scrobblingOnChanged(bool)), SLOT(update()));

    onActionsChanged();

    m_volumeSlider = new VolumeSlider( RadioService::instance().audioOutput(), this );
    m_volumeSlider->hide();

    m_volumeSlider->setAttribute( Qt::WA_Hover );
    m_volumeSlider->installEventFilter( this );
    ui->volume->setAttribute( Qt::WA_Hover );
    ui->volume->installEventFilter( this );

    // if our buttons are pressed we should trigger the actions
    connect( ui->love, SIGNAL(clicked()), aApp->loveAction(), SLOT(trigger()));
    connect( ui->ban, SIGNAL(clicked()), aApp->banAction(), SLOT(trigger()));
    connect( ui->play, SIGNAL(clicked()), aApp->playAction(), SLOT(trigger()));
    connect( ui->skip, SIGNAL(clicked()), aApp->skipAction(), SLOT(trigger()));
    connect( ui->volume, SIGNAL(clicked()), SLOT(mute()));

    connect( &ScrobbleService::instance(), SIGNAL(frameChanged(int)), SLOT(onFrameChanged(int)) );
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
    style()->polish( ui->play );
    style()->polish( ui->skip );
    style()->polish( ui->love );
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
    setUpdatesEnabled( false );

    ui->icon->setPixmap( QPixmap( ":/control_bar_radio_as.png" ) );

    ui->status->setText( tr("Tuning") );
    ui->device->setText( station.title().isEmpty() ? tr( "A Radio Station" ) : station.title() );

    ui->play->setChecked( true );
    aApp->playAction()->setChecked( true );
    ui->play->setChecked( false );

    aApp->playAction()->setChecked( false );
    aApp->playAction()->setEnabled( true );
    aApp->loveAction()->setEnabled( false );
    aApp->banAction()->setEnabled( false );
    aApp->skipAction()->setEnabled( false );
    aApp->tagAction()->setEnabled( false );
    aApp->shareAction()->setEnabled( false );

    ui->buttons->setVisible( true );
    ui->love->setVisible( true );
    ui->ban->setVisible( true );
    ui->as->setVisible( true );
    ui->time->setVisible( false );
    ui->time->setText( "" );
    ui->scrobbleMeter->setVisible( true );
    ui->scrobbleMeter->setRange( 0, 1 );
    ui->scrobbleMeter->setValue( 0 );
    ui->scrobbleMeter->setToolTip( tr( "Scrobble meter: %1%" ).arg( 0 ) );
    ui->progress->setVisible( false );
    ui->progress->setRange( 0, 1 );
    ui->progress->setValue( 0 );
    ui->progressSpacer->setVisible( true );
    ui->message->setVisible( false );
    ui->message->setText( "" );
    ui->as->setVisible( true );
    ui->as->setPixmap( QPixmap( ":/scrobble_marker_OFF.png" ) );
    ui->as->setToolTip( tr( "Not scrobbled" ) );
    ui->volumeFrame->setVisible( true );

    setScrobbleTrack( false );

    setUpdatesEnabled( true );
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
    aApp->banAction()->setEnabled( false );
    aApp->skipAction()->setEnabled( false );
    aApp->tagAction()->setEnabled( false );
    aApp->shareAction()->setEnabled( false );
}

void
PlaybackControlsWidget::setTime( int frame, const lastfm::Track& track )
{
    QTime duration( 0, 0 );
    duration = duration.addMSecs( track.duration() * 1000 );
    QTime progress( 0, 0 );
    progress = progress.addMSecs( frame );

    QString format( duration.hour() > 0 ? "h:mm:ss" : "m:ss" );

    if ( m_track.source() == Track::LastFmRadio )
        ui->time->setText( QString( "%1 / %2" ).arg( progress.toString( format ), duration.toString( format ) ) );
    else
        ui->time->setText( QString( "%1" ).arg( progress.toString( format ) ) );
}


void
PlaybackControlsWidget::setTrack( const Track& track )
{
    // we're about to change loads of stuff to don't update until the end
    setUpdatesEnabled( false );

    onVolumeChanged( RadioService::instance().audioOutput()->volume() );
    m_volumeSlider->setAudioOutput( RadioService::instance().audioOutput() );

    disconnect( m_track.signalProxy(), SIGNAL(loveToggled(bool)), ui->love, SLOT(setChecked(bool)));
    disconnect( m_track.signalProxy(), SIGNAL(scrobbleStatusChanged(short)), this, SLOT(onScrobbleStatusChanged(short)) );
    m_track = track;
    connect( m_track.signalProxy(), SIGNAL(scrobbleStatusChanged(short)), this, SLOT(onScrobbleStatusChanged(short)) );

    setTime( 0, track );

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

        aApp->playAction()->setChecked( track.source() == Track::LastFmRadio );

        // can only ban and skip radio tracks
        aApp->banAction()->setEnabled( track.source() == Track::LastFmRadio );
        aApp->skipAction()->setEnabled( track.source() == Track::LastFmRadio );

        aApp->loveAction()->setChecked( track.isLoved() );

        bool radioTrack = track.source() == Track::LastFmRadio && !m_track.url().isLocalFile();
        bool externalPlayer = track.extra( "playerId" ) == "spt" || track.extra( "playerId" ) == "mpris2";
        bool scrobbleTrack = track.source() != Track::LastFmRadio && !externalPlayer;

        ui->buttons->setVisible( radioTrack );

        ui->love->setVisible( true ); // you can always love a track

        // we show back instead of ban only for local playlist tracks
        ui->ban->setEnabled( radioTrack ); // disable when it's not radio

        ui->volumeFrame->setVisible( radioTrack );

        ui->as->setVisible( radioTrack || scrobbleTrack );
        ui->as->setToolTip( tr( "Not scrobbled" ) );
        ui->time->setVisible( radioTrack );

        ui->scrobbleMeter->setVisible( radioTrack || scrobbleTrack );
        ui->scrobbleMeter->setRange( 0, ScrobbleService::instance().stopWatch()->scrobblePoint() * 1000 );
        ui->scrobbleMeter->setValue( 0 );
        ui->scrobbleMeter->setToolTip( tr( "Scrobble meter: %1%" ).arg( 0 ) );

        ui->progress->setVisible( radioTrack );
        ui->progress->setRange( 0, m_track.duration() * 1000 );
        ui->progress->setValue( 0 );
        ui->progressSpacer->setVisible( !radioTrack );

        connect( track.signalProxy(), SIGNAL(loveToggled(bool)), ui->love, SLOT(setChecked(bool)));

        ui->status->setText( externalPlayer || radioTrack ? tr("Listening to") : tr("Scrobbling from") );

        setScrobbleTrack( !radioTrack );

        ui->message->setVisible( externalPlayer );
        ui->message->setText( track.extra( "playerId" ) == "spt" ?
                                  unicorn::Label::boldLinkStyle( tr( "Enable scrobbling by getting the %1." ).arg( unicorn::Label::anchor( "spotify:app:lastfm:route:login", tr( "Last.fm app for Spotify" ) ) ), Qt::black ):
                                  "" );

        if ( radioTrack )
        {
            QString title = RadioService::instance().station().title();
            ui->device->setText( title.isEmpty() ? tr( "A Radio Station" ) : title );
        }
        else
        {
            // Not a radio track
            ui->device->setText( track.extra( "playerName" ) );
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
    else
    {
        // what do we do with a null track???
    }

    setUpdatesEnabled( true );
}


void
PlaybackControlsWidget::onVolumeChanged( qreal volume )
{
    QPixmap volumePixmap;

    if ( RadioService::instance().audioOutput()->isMuted() )
        volumePixmap.load( ":/volume_mute.png" );
    else if ( volume < 0.3 )
        volumePixmap.load( ":/volume_low.png" );
    else if ( volume < 0.7 )
        volumePixmap.load( ":/volume_mid.png" );
    else
        volumePixmap.load( ":/volume_high.png" );

    ui->volume->setIconSize( volumePixmap.size() );
    ui->volume->setIcon(  QIcon( volumePixmap ) );
}

bool
PlaybackControlsWidget::eventFilter( QObject *obj, QEvent *event )
{
    if ( !m_volumeHideTimer )
    {
        m_volumeHideTimer = new QTimer( this );
        m_volumeHideTimer->setInterval( 500 );
        m_volumeHideTimer->setSingleShot( true );
        connect( m_volumeHideTimer, SIGNAL(timeout()), m_volumeSlider, SLOT(animateClose()) );
    }

    if ( event->type() == QEvent::Enter )
    {
        m_volumeHideTimer->stop();

        if ( obj == ui->volume && !m_volumeSlider->isVisible() )
        {
            QPoint topLeft( ui->volumeSplitter2->geometry().topLeft() );
            topLeft = ui->volume->parentWidget()->mapTo( this, topLeft );
            QRect geo( topLeft, QSize( m_volumeSlider->size().width(), ui->volume->height() ) );
            m_volumeSlider->setGeometry( geo );

            m_volumeSlider->animateOpen();
        }
    }
    else if ( event->type() == QEvent::Leave )
    {
        if ( !m_volumeSlider->underMouse() && !ui->volume->underMouse() )
            m_volumeHideTimer->start();
    }

    return QFrame::eventFilter( obj, event );
}

void
PlaybackControlsWidget::mute()
{
    qreal volume = RadioService::instance().audioOutput()->volume();
    RadioService::instance().audioOutput()->setMuted( !RadioService::instance().audioOutput()->isMuted() );
    RadioService::instance().audioOutput()->setVolume( volume );
    onVolumeChanged( RadioService::instance().audioOutput()->volume() );
}


void
PlaybackControlsWidget::onFrameChanged( int frame )
{
    if ( ui->scrobbleMeter->maximum() != 1 )
    {
        if ( m_track.source() == Track::LastFmRadio && RadioService::instance().mediaObject() )
            setTime( RadioService::instance().mediaObject()->currentTime(), m_track );
        else
            setTime( frame, m_track );

        ui->progress->setValue( frame >= ui->progress->maximum() ? ui->progress->maximum() : frame );
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
