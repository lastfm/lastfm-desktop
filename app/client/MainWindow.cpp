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

#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QSizeGrip>
#include <QTimer>
#include <QMenuBar>
#include <QShortcut>
#include <QToolBar>
#include <QDockWidget>
#include <QScrollArea>
#include <QNetworkReply>

#include "MainWindow.h"


#include "Application.h"
#include "Services/RadioService.h"
#include "Services/ScrobbleService.h"
#include "Services/AnalyticsService.h"
#include "MediaDevices/DeviceScrobbler.h"
#include "Dialogs/CloseAppsDialog.h"
#include "../Widgets/ProfileWidget.h"
#include "../Widgets/FriendListWidget.h"
#include "../Widgets/ScrobbleControls.h"
#include "../Widgets/NowPlayingStackedWidget.h"
#include "../Widgets/ScrobblesWidget.h"
#include "../Widgets/SideBar.h"
#include "../Widgets/StatusBar.h"
#include "../Widgets/TitleBar.h"
#include "../Widgets/PlaybackControlsWidget.h"
#include "../Widgets/RadioWidget.h"
#include "../Widgets/NowPlayingWidget.h"
#include "lib/unicorn/widgets/DataBox.h"
#include "lib/unicorn/widgets/MessageBar.h"
#include "lib/unicorn/widgets/GhostWidget.h"
#include "lib/unicorn/widgets/UserToolButton.h"
#include "lib/unicorn/widgets/MessageBar.h"
#include "lib/unicorn/widgets/UserMenu.h"
#include "lib/unicorn/qtwin.h"
#include "lib/unicorn/layouts/SlideOverLayout.h"
#include "lib/unicorn/widgets/SlidingStackedWidget.h"
#include "lib/listener/PlayerConnection.h"
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include "lib/unicorn/Updater/Updater.h"
#endif
#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/DesktopServices.h"

#ifdef Q_OS_WIN32
#include "../Plugins/PluginList.h"
#endif

#ifdef Q_OS_MAC
void qt_mac_set_dock_menu(QMenu *menu);
#endif

const QString CONFIG_URL = "http://cdn.last.fm/client/config.xml";

MainWindow::MainWindow( QMenuBar* menuBar )
    :unicorn::MainWindow( menuBar )
{
    hide();

#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac( true );
#else
    setMenuBar( menuBar );
#endif
    
    setCentralWidget(new QWidget);

    QVBoxLayout* layout = new QVBoxLayout( centralWidget() );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );

    layout->addWidget( ui.messageBar = new MessageBar( this ) );

    QHBoxLayout* h = new QHBoxLayout();
    h->setContentsMargins( 0, 0, 0, 0 );
    h->setSpacing( 0 );

    layout->addLayout( h );

    h->addWidget( ui.sideBar = new SideBar( this ) );

    h->addWidget( ui.stackedWidget = new unicorn::SlidingStackedWidget( this ) );

    connect( ui.sideBar, SIGNAL(currentChanged(int)), ui.stackedWidget, SLOT(slide(int)));

    ui.stackedWidget->addWidget( ui.nowPlaying = new NowPlayingStackedWidget(this) );
    ui.nowPlaying->setObjectName( "nowPlaying" );

    ui.stackedWidget->addWidget( ui.scrobbles = new ScrobblesWidget( this ) );
    ui.scrobbles->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    connect( ui.stackedWidget, SIGNAL(currentChanged(int)), ui.scrobbles, SLOT(onCurrentChanged(int)) );

    ui.stackedWidget->addWidget( ui.profileScrollArea = new QScrollArea( this ) );
    ui.profileScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui.profileScrollArea->setWidget( ui.profile = new ProfileWidget(this) );
    ui.profileScrollArea->setWidgetResizable( true );
    ui.profile->setObjectName( "profile" );

    connect( ui.stackedWidget, SIGNAL(currentChanged(int)), ui.profile, SLOT(onCurrentChanged(int)) );

    ui.stackedWidget->addWidget( ui.friends = new FriendListWidget(this) );
    ui.friends->setObjectName( "friends" );

    connect( ui.stackedWidget, SIGNAL(currentChanged(int)), ui.friends, SLOT(onCurrentChanged(int)) );


    ui.stackedWidget->addWidget( ui.radioScrollArea = new QScrollArea( this ) );
    ui.radioScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui.radioScrollArea->setWidget( ui.radio = new RadioWidget( this ) );
    ui.radioScrollArea->setWidgetResizable( true );
    ui.radio->setObjectName( "radio" );

    ui.statusBar = new StatusBar( this );
    ui.statusBar->setObjectName( "StatusBar" );

    ui.statusBar->setSizeGripEnabled( false );

    setWindowTitle( applicationName() );
    setUnifiedTitleAndToolBarOnMac( true );

    connect( &ScrobbleService::instance(), SIGNAL( trackStarted(Track, Track) ), SLOT( onTrackStarted(Track, Track) ) );
    connect( &ScrobbleService::instance(), SIGNAL( paused() ), SLOT( onPaused() ) );
    connect( &ScrobbleService::instance(), SIGNAL( resumed() ), SLOT( onResumed() ) );
    connect( &ScrobbleService::instance(), SIGNAL( stopped() ), SLOT( onStopped() ) );

    connect( &RadioService::instance(), SIGNAL(tuningIn(RadioStation)), SLOT(onTuningIn()));
    connect( &RadioService::instance(), SIGNAL(error(int,QVariant)), SLOT(onRadioError(int,QVariant)));
    connect( &RadioService::instance(), SIGNAL(message(const QString&)), SLOT(onRadioMessage(const QString&)));

    connect( &ScrobbleService::instance(), SIGNAL(foundIPodScrobbles(QList<lastfm::Track>)), SLOT(onFoundScrobbles(QList<lastfm::Track>)));

    new QShortcut( Qt::Key_Space, this, SLOT(onSpace()) );

    //for some reason some of the stylesheet is not being applied properly unless reloaded
    //here. StyleSheets see very flaky to me. :s
    aApp->refreshStyleSheet();

    setMinimumWidth( 540 );
    setMaximumWidth( 800 );

    setStatusBar( ui.statusBar );

    // This is the default window size it will get changed
    // by finishUi if the app has ever been opened before
    resize( 565, 710 );
    finishUi();

#ifdef Q_OS_WIN32
    m_pluginList = new PluginList( this );

    QTimer::singleShot( 1000, this, SLOT(checkUpdatedPlugins()) );

#endif

    setupMenuBar();

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    m_updater = new Updater( this );
#endif

#ifdef Q_OS_MAC
    QMenu* dockMenu = new QMenu();
    ui.nowPlaying->nowPlaying()->playbackControls()->addToMenu( *dockMenu  );
    qt_mac_set_dock_menu( dockMenu );
#endif

    if (aApp->tray())
    {
        ui.nowPlaying->nowPlaying()->playbackControls()->addToMenu( *aApp->tray()->contextMenu(), aApp->tray()->contextMenu()->actions()[3] );
    }

    connect( lastfm::nam()->get( QNetworkRequest( CONFIG_URL ) ), SIGNAL(finished()), SLOT(onConfigRetrieved()) );
}

void
MainWindow::showMessage( const QString& message, const QString& id, int timeout )
{
    ui.messageBar->show( message, id, timeout );
}

QString
MainWindow::applicationName()
{
    return QCoreApplication::applicationName();
}

#ifdef Q_OS_WIN32
void
MainWindow::checkUpdatedPlugins()
{
    if ( m_pluginList->updatedList().count() > 0 )
    {
        // one of the plugins has been updated so ask if they want to install them

        if ( QMessageBoxBuilder( this ).setText( tr( "There are updates to your media player plugins. Would you like to install them now?" ) )
             .setTitle( "Updates to media player plugins" )
             .setButtons( QMessageBox::Yes | QMessageBox::No )
             .exec() == QMessageBox::Yes )
        {
            CloseAppsDialog* closeApps = new CloseAppsDialog( m_pluginList->updatedList(), this );

            if ( closeApps->result() != QDialog::Accepted )
                closeApps->exec();
            else
                closeApps->deleteLater();

            if ( closeApps->result() == QDialog::Accepted )
            {
                foreach ( IPluginInfo* info, m_pluginList->updatedList() )
                {
                    info->setVerbose( false );
                    info->doInstall();
                    info->setVerbose( true );
                }

                // The user didn't close their media players
                QMessageBoxBuilder( this ).setTitle( tr( "Plugin(s) installed!", "", m_pluginList->updatedList().count() ) )
                        .setIcon( QMessageBox::Information )
                        .setText( tr( "<p>Your plugin(s) ha(s|ve) been installed.</p>"
                                      "<p>You're now ready to scrobble with your media player(s)</p>", "", m_pluginList->updatedList().count() ) )
                        .setButtons( QMessageBox::Ok )
                        .exec();
            }
            else
            {
                // The user didn't close their media players
                QMessageBoxBuilder( this ).setTitle( tr( "Your plugins haven't been installed" ) )
                        .setIcon( QMessageBox::Warning )
                        .setText( tr( "You can install them later through the file menu" ) )
                        .setButtons( QMessageBox::Ok )
                        .exec();
            }
        }
    }
}
#endif

QString MainWindow::currentCategory() const
{
    return ui.sideBar->currentCategory();
}

void
MainWindow::setupMenuBar()
{
    /// File menu (should only show on non-mac)
    QMenu* fileMenu = appMenuBar()->addMenu( tr( "File" ) );

#ifdef Q_OS_WIN32
    QMenu* pluginMenu = fileMenu->addMenu( tr( "Install plugins" ) );

    foreach ( IPluginInfo* info, m_pluginList->supportedList() )
    {
        info->setVerbose( true );
        pluginMenu->addAction( info->name(), info, SLOT(doInstall()));
    }
#endif

    QAction* quit = fileMenu->addAction( tr("&Quit"), qApp, SLOT(quit()) );
    quit->setMenuRole( QAction::QuitRole );
#ifdef Q_OS_WIN
    quit->setShortcut( Qt::ALT + Qt::Key_F4 );
#else
    quit->setShortcut( Qt::CTRL + Qt::Key_Q );
#endif

    /// View
    QMenu* viewMenu = appMenuBar()->addMenu( tr("View") );
    ui.sideBar->addToMenu( *viewMenu );
    viewMenu->addSeparator();
    viewMenu->addAction( tr( "My Last.fm Profile" ), this, SLOT(onVisitProfile()), Qt::CTRL + Qt::Key_P );

    /// Scrobbles
    QMenu* scrobblesMenu = appMenuBar()->addMenu( tr("Scrobbles") );
    scrobblesMenu->addAction( tr( "Refresh" ), ui.scrobbles, SLOT(refresh()), Qt::CTRL + Qt::SHIFT + Qt::Key_R );

    /// Controls
    QMenu* controlsMenu = appMenuBar()->addMenu( tr("Controls") );
    ui.nowPlaying->nowPlaying()->playbackControls()->addToMenu( *controlsMenu  );

    /// Account
    appMenuBar()->addMenu( new UserMenu( this ) )->setText( tr( "Account" ) );

    /// Tools (should only show on non-mac)
    QMenu* toolsMenu = appMenuBar()->addMenu( tr("Tools") );
#ifndef Q_WS_X11
    QAction* c4u = toolsMenu->addAction( tr("Check for Updates"), this, SLOT(checkForUpdates()) );
    c4u->setMenuRole( QAction::ApplicationSpecificRole );
#endif
    QAction* prefs = toolsMenu->addAction( tr("Options"), this, SLOT(onPrefsTriggered()) );
    prefs->setMenuRole( QAction::PreferencesRole );

    /// Window
    QMenu* windowMenu = appMenuBar()->addMenu( tr("Window") );
    windowMenu->addAction( tr( "Minimize" ), this, SLOT(onMinimizeTriggered()), Qt::CTRL + Qt::Key_M );
    windowMenu->addAction( tr( "Zoom" ), this, SLOT(onZoomTriggered()) );
    //windowMenu->addSeparator();
    //windowMenu->addAction( tr( "Last.fm" ), this, SLOT(onZoomTriggered()) );
    windowMenu->addSeparator();
    windowMenu->addAction( tr( "Bring All to Front" ), this, SLOT(onBringAllToFrontTriggered()) );

    /// Help
    QMenu* helpMenu = appMenuBar()->addMenu( tr("Help") );
    QAction* about = helpMenu->addAction( tr("About"), aApp, SLOT(onAboutTriggered()) );
    about->setMenuRole( QAction::AboutRole );
    helpMenu->addSeparator();
    helpMenu->addAction( tr("FAQ"), aApp, SLOT(onFaqTriggered()) );
    helpMenu->addAction( tr("Forums"), aApp, SLOT(onForumsTriggered()) );
    helpMenu->addSeparator();
    helpMenu->addAction( tr("Tour"), aApp, SLOT(onTourTriggered()) );
    helpMenu->addSeparator();
    helpMenu->addAction( tr("Show Licenses"), aApp, SLOT(onLicensesTriggered()) );
#ifndef Q_WS_X11 // it's only the scrobble log tab at the moment so no use on linux
    helpMenu->addSeparator();
    helpMenu->addAction( tr("Diagnostics"), aApp, SLOT(onDiagnosticsTriggered()) );
#endif
}

void
MainWindow::onSpace()
{
    aApp->playAction()->trigger();
}

void
MainWindow::onConfigRetrieved()
{
    XmlQuery xq;

    if( xq.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {
        // -- grab the song count and set it for playback.
        int songCount = xq["songcount"].text().toInt();

        if(songCount > 0)
            RadioService::instance().setMaxUsageCount( songCount );

        // -- grab the message and display it on load
        QString message = xq["message"]["text"].text();

        if ( !message.isEmpty() )
            onRadioMessage( message );
    }
}

void
MainWindow::onVisitProfile()
{
    unicorn::DesktopServices::openUrl( aApp->currentSession().user().www() );
    AnalyticsService::instance().sendEvent( aApp->currentCategory(), LINK_CLICKED, "ProfileURLClicked");
}

void
MainWindow::showEvent(QShowEvent *)
{
    if ( m_preferences )
        m_preferences->show();

    m_menuBar->show();
    m_menuBar->activateWindow();

#ifdef Q_OS_MAC
    if ( !m_installer )
    {
        m_installer = new ITunesPluginInstaller( this );
        QTimer::singleShot( 1000, m_installer, SLOT(install()) );
    }
#endif
}

void
MainWindow::hideEvent(QHideEvent *)
{
    if ( m_preferences )
        m_preferences->hide();
}

void
MainWindow::onPrefsTriggered()
{
    if ( !m_preferences )
        m_preferences = new PreferencesDialog( 0, this );

    m_preferences->show();
    m_preferences->activateWindow();
    m_preferences->adjustSize();

    AnalyticsService::instance().sendEvent( aApp->currentCategory(), BASIC_SETTINGS, "SettingsOpened");
}

void
MainWindow::onDiagnosticsTriggered()
{
    if ( !m_diagnostics )
        m_diagnostics = new DiagnosticsDialog( this );

    m_diagnostics->show();
    m_diagnostics->activateWindow();
}

void
MainWindow::setBetaUpdates( bool betaUpdates )
{
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    if ( m_updater )
        m_updater->setBetaUpdates( betaUpdates );
#endif
}

void
MainWindow::onMinimizeTriggered()
{
    setWindowState( Qt::WindowMinimized );
}

void
MainWindow::onZoomTriggered()
{
    setWindowState( Qt::WindowMaximized );
}

void
MainWindow::onBringAllToFrontTriggered()
{
	return;
}

void
MainWindow::checkForUpdates()
{
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    m_updater->checkForUpdates();
#endif
}

void
MainWindow::onTuningIn()
{
    /* 0 is the now playing widget in the stack */
    ui.sideBar->click( 0 );
}

void
MainWindow::onTrackStarted( const Track& t, const Track& /*previous*/ )
{
    m_currentTrack = t;

    if ( m_currentTrack.source() == Track::LastFmRadio )
        setWindowTitle( tr( "%1 - %2 - %3" ).arg( t.toString(), RadioService::instance().station().title(), applicationName() ) );
    else
        setWindowTitle( tr( "%1 - %2" ).arg( t.toString(), applicationName() ) );
}


void
MainWindow::onStopped()
{
    m_currentTrack = Track();

    setWindowTitle( applicationName() );
}


void
MainWindow::onResumed()
{
    if ( m_currentTrack.source() == Track::LastFmRadio )
        setWindowTitle( tr( "%1 - %2 - %3" ).arg( m_currentTrack.toString(), RadioService::instance().station().title(), applicationName() ) );
    else
        setWindowTitle( tr( "%1 - %2" ).arg( m_currentTrack.toString(), applicationName() ) );
}


void
MainWindow::onPaused()
{
    if ( m_currentTrack.source() == Track::LastFmRadio )
        setWindowTitle( tr( "%1 - %2" ).arg( RadioService::instance().station().title(), applicationName() ) );
    else
        setWindowTitle( tr( "%1" ).arg( applicationName() ) );
}


void
MainWindow::onRadioError( int error, const QVariant& data )
{
    ui.messageBar->show( tr( "%1: %2" ).arg( data.toString(), QString::number( error ) ), "radio" );
}

void
MainWindow::onRadioMessage(const QString &message)
{
    ui.messageBar->show(message, "radio");
}

void
MainWindow::onFoundScrobbles( const QList<lastfm::Track>& tracks )
{
    if ( tracks.count() > 0 )
    {
        ui.messageBar->addTracks( tracks );

        int count = 0;

        foreach ( const lastfm::Track& track, ui.messageBar->tracks() )
            count += track.extra( "playCount" ).toInt();

        ui.messageBar->show( tr( "<a href=\"tracks\">%n play(s)</a> ha(s|ve) been scrobbled from a device", "", count ), "ipod" );
    }
}

void
MainWindow::addWinThumbBarButton( QAction* thumbButtonAction )
{
    m_buttons.append( thumbButtonAction );
}


void
MainWindow::addWinThumbBarButtons( QList<QAction*>& thumbButtonActions )
{
    foreach ( QAction* button, m_buttons )
        thumbButtonActions.append( button );
}

