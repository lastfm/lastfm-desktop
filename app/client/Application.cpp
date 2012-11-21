/*
   Copyright 2005-2010 Last.fm Ltd. 
      - Primarily authored by Jono Cole and Michael Coffey

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

#include <QRegExp>
#include <QShortcut>
#include <QFileDialog>
#include <QDesktopServices>
#include <QNetworkDiskCache>
#include <QMenu>
#include <QMenuBar>
#include <QDebug>
#include <QProcess>
#include <QShortcut>
#include <QTcpSocket>
#include <QAction>

#include <lastfm/UrlBuilder.h>
#include <lastfm/InternetConnectionMonitor.h>
#include <lastfm/XmlQuery.h>
#include <lastfm/misc.h>

#include "lib/listener/State.h"
#include "lib/listener/PlayerConnection.h"
#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/dialogs/AboutDialog.h"
#include "lib/unicorn/dialogs/ShareDialog.h"
#include "lib/unicorn/UnicornSession.h"
#include "lib/unicorn/dialogs/TagDialog.h"
#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/widgets/UserMenu.h"
#include "lib/unicorn/DesktopServices.h"
#ifdef Q_OS_MAC
#include "MediaKeys/MediaKey.h"
#include "lib/unicorn/notify/Notify.h"
#include "CommandReciever/CommandReciever.h"
#endif

#include "Fingerprinter/Fingerprinter.h"
#include "Dialogs/LicensesDialog.h"
#include "MediaDevices/DeviceScrobbler.h"
#include "Services/RadioService.h"
#include "Services/ScrobbleService.h"
#include "Services/AnalyticsService.h"
#include "Widgets/PointyArrow.h"
#include "Widgets/ScrobbleControls.h"
#include "Widgets/MetadataWidget.h"
#include "Wizard/FirstRunWizard.h"
#include "Application.h"
#include "MainWindow.h"
#include "AudioscrobblerSettings.h"

#ifdef Q_OS_WIN32
#include "windows.h"
#endif

using audioscrobbler::Application;

#define ELLIPSIS QString::fromUtf8("…")
#define CONTROL_KEY_CHAR QString::fromUtf8("⌃")
#define APPLE_KEY_CHAR QString::fromUtf8("⌘")

#ifdef Q_WS_X11
    #define AS_TRAY_ICON ":22x22.png"
#elif defined( Q_WS_WIN )
    #define AS_TRAY_ICON ":16x16.png"
#elif defined( Q_WS_MAC )
    #define AS_TRAY_ICON ":systray_icon_rest_mac.png"
#endif

Application::Application(int& argc, char** argv) 
    :unicorn::Application(argc, argv), m_raiseHotKeyId( (void*)-1 )
{
#ifdef Q_OS_MAC
    m_mediaKey = new MediaKey( this );
#endif
}

void
Application::initiateLogin( bool forceWizard ) throw( StubbornUserException )
{
    closeAllWindows();

    if( forceWizard || !unicorn::Settings().value( SETTING_FIRST_RUN_WIZARD_COMPLETED, false ).toBool() )
    {
        setWizardRunning( true );

        FirstRunWizard w;
        if( w.exec() != QDialog::Accepted )
        {
            setWizardRunning( false );
            throw StubbornUserException();
        }

        setWizardRunning( false );
    }

    //this covers the case where the last user was removed
    //and the main window was closed.
    if ( m_mw )
        m_mw->show();

    if ( m_tray )
    {
        //HACK: turns out when all the windows are closed, the tray stops working
        //unless you call the following methods.
        m_tray->hide();
        m_tray->show();
    }
}

void
Application::init()
{
    // Initialise the unicorn base class first!
    unicorn::Application::init();

#ifdef Q_WS_X11
    setWindowIcon( QIcon( ":/as.png" ) );
#endif

    if ( !currentSession() )
    {
        // there won't be a current session if one wasn't created by the wizard

        QMap<QString, QString> lastSession = unicorn::Session::lastSessionData();
        if ( lastSession.contains( "username" ) && lastSession.contains( "sessionKey" ) )
            changeSession( lastSession[ "username" ], lastSession[ "sessionKey" ] );
    }

    initiateLogin( !currentSession() );

//    QNetworkDiskCache* diskCache = new QNetworkDiskCache(this);
//    diskCache->setCacheDirectory( lastfm::dir::cache().path() );
//    lastfm::nam()->setCache( diskCache );

/// tray
    tray(); // this will initialise m_tray if it doesn't already exist

    /// tray menu
    QMenu* menu = new QMenu;
    m_tray->setContextMenu(menu);

    menu->addMenu( new UserMenu() )->setText( "Accounts" );

    m_show_window_action = menu->addAction( tr("Show Scrobbler"));
    m_show_window_action->setShortcut( Qt::CTRL + Qt::META + Qt::Key_S );
    menu->addSeparator();

    {
        m_love_action = new QAction( tr("Love"), this );
        m_love_action->setIconVisibleInMenu( false );
        m_love_action->setCheckable( true );
        QIcon loveIcon;
        loveIcon.addFile( ":/meta_love_OFF_REST.png", QSize( 16, 16 ), QIcon::Normal, QIcon::Off );
        loveIcon.addFile( ":/meta_love_ON_REST.png", QSize( 16, 16 ), QIcon::Normal, QIcon::On );

        m_love_action->setIcon( loveIcon );
        m_love_action->setEnabled( false );
        connect( m_love_action, SIGNAL(triggered(bool)), SLOT(changeLovedState(bool)));
    }
    {
        m_play_action = new QAction( tr( "Play" ), this );
        m_play_action->setIconVisibleInMenu( false );
        m_play_action->setCheckable( true );
        QIcon playIcon;
        playIcon.addFile( ":/controls_pause_REST.png", QSize(), QIcon::Normal, QIcon::On );
        playIcon.addFile( ":/controls_play_REST.png", QSize(), QIcon::Normal, QIcon::Off );
        m_play_action->setIcon( playIcon );
    }
    {
        m_skip_action = new QAction( tr( "Skip" ), this );
        m_skip_action->setIconVisibleInMenu( false );
        QIcon skipIcon;
        skipIcon.addFile( ":/controls_skip_REST.png" );
        m_skip_action->setIcon( skipIcon );
        m_skip_action->setEnabled( false );
    }
    {
        m_tag_action = new QAction( tr( "Tag" ) + ELLIPSIS, this );
        m_tag_action->setIconVisibleInMenu( false );
        m_tag_action->setIcon( QIcon( ":/meta_tag_REST.png" ) );
        m_tag_action->setEnabled( false );
        connect( m_tag_action, SIGNAL(triggered()), SLOT(onTagTriggered()));
    }
    {
        m_share_action = new QAction( tr( "Share" ) + ELLIPSIS, this );
        m_share_action->setIconVisibleInMenu( false );
        m_share_action->setIcon( QIcon( ":/meta_share_REST.png" ) );
        m_share_action->setEnabled( false );
        connect( m_share_action, SIGNAL(triggered()), SLOT(onShareTriggered()));
    }
    {
        m_ban_action = new QAction( tr( "Ban" ), this );
        m_ban_action->setIconVisibleInMenu( false );
        QIcon banIcon;
        banIcon.addFile( ":/controls_ban_REST.png" );
        m_ban_action->setIcon( banIcon );
        m_ban_action->setEnabled( false );
    }
    {
        m_mute_action = new QAction( tr( "Mute" ), this );
        m_mute_action->setEnabled( true );
    }


#ifdef Q_WS_X11
    menu->addSeparator();
    m_scrobble_ipod_action = menu->addAction( tr( "Scrobble iPod..." ) );
    connect( m_scrobble_ipod_action, SIGNAL( triggered() ), ScrobbleService::instance().deviceScrobbler(), SLOT( onScrobbleIpodTriggered() ) );
#endif

    menu->addSeparator();

    m_visit_profile_action = menu->addAction( tr( "Visit Last.fm profile" ) );
    connect( m_visit_profile_action, SIGNAL( triggered() ), SLOT( onVisitProfileTriggered() ) );

    menu->addSeparator();

    m_submit_scrobbles_toggle = menu->addAction( tr("Enable Scrobbling") );
    m_submit_scrobbles_toggle->setCheckable( true );
    bool scrobblingOn = unicorn::UserSettings().value( "scrobblingOn", true ).toBool();
    m_submit_scrobbles_toggle->setChecked( scrobblingOn );
    ScrobbleService::instance().scrobbleSettingsChanged();

    connect( m_submit_scrobbles_toggle, SIGNAL(toggled(bool)), SLOT(onScrobbleToggled(bool)) );
    connect( this, SIGNAL(scrobbleToggled(bool)), &ScrobbleService::instance(), SLOT(scrobbleSettingsChanged()) );

    menu->addSeparator();

    QAction* quit = menu->addAction(tr("Quit %1").arg( applicationName()));
    connect(quit, SIGNAL(triggered()), SLOT(quit()));

    m_menuBar = new QMenuBar( 0 );

/// MainWindow
    m_mw = new MainWindow( m_menuBar );
    m_mw->addWinThumbBarButton( m_love_action );
    m_mw->addWinThumbBarButton( m_ban_action );
    m_mw->addWinThumbBarButton( m_play_action );
    m_mw->addWinThumbBarButton( m_skip_action );

    m_toggle_window_action = new QAction( this ), SLOT( trigger());
#ifndef Q_WS_X11
     AudioscrobblerSettings settings;
     setRaiseHotKey( settings.raiseShortcutModifiers(), settings.raiseShortcutKey() );
#endif
    m_skip_action->setShortcut( Qt::CTRL + Qt::Key_Right );
    m_tag_action->setShortcut( Qt::CTRL + Qt::Key_T );
    m_share_action->setShortcut( Qt::CTRL + Qt::Key_S );
    m_love_action->setShortcut( Qt::CTRL + Qt::Key_L );
    m_ban_action->setShortcut( Qt::CTRL + Qt::Key_B );
    m_mute_action->setShortcut( Qt::CTRL + Qt::ALT + Qt::Key_Down );

    // make the love buttons sychronised
    connect(this, SIGNAL(lovedStateChanged(bool)), m_love_action, SLOT(setChecked(bool)));

    // tell the radio that the scrobbler's love state has changed
    connect(this, SIGNAL(lovedStateChanged(bool)), SLOT(sendBusLovedStateChanged(bool)));

    // update the love buttons if love was pressed in the radio
    connect(this, SIGNAL(busLovedStateChanged(bool)), m_love_action, SLOT(setChecked(bool)));
    connect(this, SIGNAL(busLovedStateChanged(bool)), SLOT(onBusLovedStateChanged(bool)));

    // tell everyone that is interested that data about the current track has been fetched
    connect( m_mw, SIGNAL(trackGotInfo(XmlQuery)), SIGNAL(trackGotInfo(XmlQuery)));
    connect( m_mw, SIGNAL(albumGotInfo(XmlQuery)), SIGNAL(albumGotInfo(XmlQuery)));
    connect( m_mw, SIGNAL(artistGotInfo(XmlQuery)), SIGNAL(artistGotInfo(XmlQuery)));
    connect( m_mw, SIGNAL(artistGotEvents(XmlQuery)), SIGNAL(artistGotEvents(XmlQuery)));
    connect( m_mw, SIGNAL(trackGotTopFans(XmlQuery)), SIGNAL(trackGotTopFans(XmlQuery)));
    connect( m_mw, SIGNAL(trackGotTags(XmlQuery)), SIGNAL(trackGotTags(XmlQuery)));
    connect( m_mw, SIGNAL(finished()), SIGNAL(finished()));

    connect( m_mw, SIGNAL(trackGotInfo(XmlQuery)), this, SLOT(onTrackGotInfo(XmlQuery)));

    connect( m_show_window_action, SIGNAL( triggered()), SLOT( showWindow()), Qt::QueuedConnection );
    connect( m_toggle_window_action, SIGNAL( triggered()), SLOT( toggleWindow()), Qt::QueuedConnection );

    connect( this, SIGNAL(messageReceived(QStringList)), SLOT(onMessageReceived(QStringList)) );
    connect( this, SIGNAL(sessionChanged(unicorn::Session)), &ScrobbleService::instance(), SLOT(onSessionChanged(unicorn::Session)) );

    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(Track,Track)), SLOT(onTrackStarted(Track,Track)));
    connect( &ScrobbleService::instance(), SIGNAL(paused(bool)), SLOT(onTrackPaused(bool)));

    connect( &RadioService::instance(), SIGNAL(trackSpooled(Track)), SLOT(onTrackSpooled(Track)) );

    // clicking on a system tray message should show the scrobbler
    connect( m_tray, SIGNAL(messageClicked()), m_show_window_action, SLOT(trigger()));

    // make sure cached scrobbles get submitted when the connection comes back online
    connect( m_icm, SIGNAL(up(QString)), &ScrobbleService::instance(), SLOT(submitCache()) );

#ifdef Q_OS_WIN32
    QStringList args = arguments();
#else
    QStringList args = arguments().mid( 1 );
#endif
    emit messageReceived( args );

#ifdef Q_OS_MAC
    m_notify = new Notify( this );
    connect( m_notify, SIGNAL(clicked()), SLOT(showWindow()) );
    connect( &ScrobbleService::instance(), SIGNAL(paused()), m_notify, SLOT(paused()) );
    connect( &ScrobbleService::instance(), SIGNAL(resumed()), m_notify, SLOT(resumed()) );
    connect( &ScrobbleService::instance(), SIGNAL(stopped()), m_notify, SLOT(stopped()) );

    new CommandReciever( this );
#endif
}

QSystemTrayIcon*
Application::tray()
{
    if ( !m_tray )
    {
        m_tray = new QSystemTrayIcon(this);
        QIcon trayIcon( AS_TRAY_ICON );
#ifdef Q_WS_MAC
        trayIcon.addFile( ":systray_icon_pressed_mac.png", QSize(), QIcon::Selected );
#endif

#ifdef Q_WS_WIN
        connect( m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT( onTrayActivated(QSystemTrayIcon::ActivationReason)) );
#endif

#ifdef Q_WS_X11
        connect( m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT( onTrayActivated(QSystemTrayIcon::ActivationReason)) );
#endif
        m_tray->setIcon(trayIcon);
        showAs( unicorn::Settings().value( SETTING_SHOW_AS, true ).toBool() );
        connect( this, SIGNAL( aboutToQuit()), m_tray, SLOT( hide()));
    }

    return m_tray;
}

void
Application::showAs( bool showAs )
{
    m_tray->setVisible( showAs  );
#ifdef Q_OS_MAC
    setQuitOnLastWindowClosed( false );
#else
    setQuitOnLastWindowClosed( !showAs && !QSystemTrayIcon::isSystemTrayAvailable() );
#endif
}


void
Application::setRaiseHotKey( Qt::KeyboardModifiers mods, int key )
{
    if( m_raiseHotKeyId >= 0 )
        unInstallHotKey( m_raiseHotKeyId );

    m_raiseHotKeyId = installHotKey( mods, key, m_toggle_window_action, SLOT(trigger()));
}

void
Application::startBootstrap( const QString& pluginId )
{
    if ( pluginId == "itw"
         || pluginId == "osx" )
        m_bootstrapper = new iTunesBootstrapper( this );
    else
        m_bootstrapper = new PluginBootstrapper( pluginId, this );

    connect( m_bootstrapper, SIGNAL(done(int)), SIGNAL(bootstrapDone(int)) );
    emit bootstrapStarted( pluginId );
    m_bootstrapper->bootStrap();
}

QString
Application::currentCategory() const
{
    return m_mw->currentCategory();
}

void
Application::onTrackGotInfo( const XmlQuery& lfm )
{
    MutableTrack( ScrobbleService::instance().currentConnection()->track() ).setFromLfm( lfm );
}


void
Application::onCorrected(QString /*correction*/)
{
    onTrackStarted( ScrobbleService::instance().currentTrack(), ScrobbleService::instance().currentTrack());
}


void
Application::onTrackStarted( const lastfm::Track& track, const Track& oldTrack )
{
    disconnect( oldTrack.signalProxy(), 0, this, 0 );

    if ( track != m_currentTrack )
    {
        m_currentTrack = track;

        if ( ScrobbleService::instance().scrobblableTrack( m_currentTrack )
             && unicorn::Settings().value( SETTING_NOTIFICATIONS, true ).toBool() )
        {
#ifdef Q_OS_MAC
            m_notify->newTrack( track );
#else
            tray()->showMessage( track.toString(), tr("from %1").arg( track.album() ) );
#endif
        }
    }

    if ( unicorn::UserSettings().value( "fingerprint", true ).toBool()
#if QT_VERSION >= 0x040800
         && track.url().isLocalFile()
#endif
       )
    {
        Fingerprinter* fingerprinter = new Fingerprinter( track, this );
        connect( fingerprinter, SIGNAL(finished()), fingerprinter, SLOT(deleteLater()) );
        fingerprinter->start();
    }

    m_tray->setToolTip( track.toString() );

    m_love_action->setEnabled( true );
    m_tag_action->setEnabled( true );
    m_share_action->setEnabled( true );

    // make sure that if the love state changes we update all the buttons
    connect( track.signalProxy(), SIGNAL(loveToggled(bool)), SIGNAL(lovedStateChanged(bool)) );
    connect( track.signalProxy(), SIGNAL(corrected(QString)), SLOT(onCorrected(QString)));
}

#ifdef Q_OS_MAC
bool
Application::macEventFilter( EventHandlerCallRef caller, EventRef event )
{
    if (!m_mediaKey)
        m_mediaKey = new MediaKey( this );

    return m_mediaKey->macEventFilter( caller, event );
}
#endif

void
Application::onTrackSpooled( const Track& /*track*/ )
{
}

void
Application::onTrackPaused( bool )
{
}

void 
Application::onTagTriggered()
{
    TagDialog* td = new TagDialog( m_currentTrack, m_mw );
    td->raise();
    td->show();
    td->activateWindow();
}

void 
Application::onShareTriggered()
{
    ShareDialog* sd = new ShareDialog( m_currentTrack, m_mw );
    sd->raise();
    sd->show();
    sd->activateWindow();
}

void
Application::onVisitProfileTriggered()
{
    unicorn::DesktopServices::openUrl( User().www() );
}

void
Application::onFaqTriggered()
{
    unicorn::DesktopServices::openUrl( lastfm::UrlBuilder( "help" ).slash( "faq" ).url() );
}

void
Application::onForumsTriggered()
{
    unicorn::DesktopServices::openUrl( lastfm::UrlBuilder( "forum" ).slash( "34905" ).url() );
}

void
Application::onTourTriggered()
{
    FirstRunWizard w( true, m_mw );
    w.exec();
}

void
Application::onAboutTriggered()
{
    if ( !m_aboutDialog )
        m_aboutDialog = new AboutDialog( m_mw );
    m_aboutDialog->show();
}

void
Application::onLicensesTriggered()
{
    if ( !m_licensesDialog )
        m_licensesDialog = new LicensesDialog( m_mw );
    m_licensesDialog->show();
}

void 
Application::changeLovedState(bool loved)
{
    MutableTrack track( m_currentTrack );

    if (loved)
        track.love();
    else
        track.unlove();
}

void
Application::onScrobbleToggled( bool scrobblingOn )
{
    if ( unicorn::UserSettings().value( "scrobblingOn", true ) != scrobblingOn )
    {
        unicorn::UserSettings().setValue( "scrobblingOn", scrobblingOn );
        AnalyticsService::instance().sendEvent(SETTINGS_CATEGORY, SCROBBLING_SETTINGS, scrobblingOn ? "ScrobbleTurnedOn" : "ScrobbleTurnedOff" );
    }

    m_submit_scrobbles_toggle->setChecked( scrobblingOn );
    emit scrobbleToggled( scrobblingOn );
}

void
Application::onBusLovedStateChanged( bool loved )
{
    MutableTrack( m_currentTrack ).setLoved( loved );
}

void 
Application::onTrayActivated( QSystemTrayIcon::ActivationReason reason ) 
{
    if( reason == QSystemTrayIcon::Context ) return;
#ifdef Q_WS_WIN
    if( reason != QSystemTrayIcon::DoubleClick ) return;
#endif
    m_show_window_action->trigger();
}

void
Application::showWindow()
{
    m_mw->showNormal();
    m_mw->setFocus();
    m_mw->raise();
    m_mw->activateWindow();
}

void
Application::toggleWindow()
{
    if( activeWindow() )
        m_mw->hide();
    else
        showWindow();
}
  

// lastfmlib invokes this directly, for some errors:
void
Application::onWsError( lastfm::ws::Error e )
{
    switch (e)
    {
        case lastfm::ws::InvalidSessionKey:
            //quit();
            // ask the current user to reauthenticate!
            break;
        default:
            break;
    }
}

  
Application::Argument Application::argument( const QString& arg )
{
    if (arg == "--pause") return Pause;
    if (arg == "--skip") return Skip;
    if (arg == "--exit") return Exit;
    if (arg == "--stop") return Stop;
    if (arg == "--twiddly") return Twiddly;
    if (arg == "--settings") return Settings;

    QUrl url( arg );
    //TODO show error if invalid schema and that
    if (url.isValid() && url.scheme() == "lastfm") return LastFmUrl;

    return ArgUnknown;
}

void
Application::onPrefsTriggered()
{
    m_mw->onPrefsTriggered();
}

void
Application::onBetaTriggered()
{
    m_mw->onBetaTriggered();
}

void
Application::onDiagnosticsTriggered()
{
    m_mw->onDiagnosticsTriggered();
}
    
void
Application::onMessageReceived( const QStringList& message )
{
    parseArguments( message );

    qDebug() << "Messages: " << message;

    if ( !( message.contains( "--tray" )
           || message.contains( "--twiddly" )
           || message.contains( "--new-ipod-detected" )
           || message.contains( "--ipod-detected" )
           || message.contains( "--settings" ) ) )
    {
        // raise the app
        m_show_window_action->trigger();
#ifdef Q_OS_WIN32
        SetForegroundWindow(m_mw->winId());
#endif
    }

}


void
Application::parseArguments( const QStringList& args )
{
    qDebug() << args;

    foreach ( QString const arg, args )
    {
        switch ( argument( arg ) )
        {
        case LastFmUrl:
            RadioService::instance().play( RadioStation( arg ) );
            break;

        case Exit:
            exit();
            break;

        case Skip:
            RadioService::instance().skip();
            break;

        case Stop:
            RadioService::instance().stop();
            break;

        case Pause:
            if ( RadioService::instance().state() == Playing )
                RadioService::instance().pause();
            else if ( RadioService::instance().state() == Paused )
                RadioService::instance().resume();
            break;

        case Twiddly:
            ScrobbleService::instance().handleTwiddlyMessage( args );
            break;

        case Settings:
            m_mw->onPrefsTriggered();
            break;

        case ArgUnknown:
            break;
        }
    }
}

void 
Application::quit()
{
    if( activeWindow() )
        activeWindow()->raise();

    if( unicorn::AppSettings().value( "quitDontAsk", false ).toBool()) {
        actuallyQuit();
        return;
    }

    bool dontAsk = false;
    int result = 1;
    if( !unicorn::AppSettings().value( "quitDontAsk", false ).toBool())
      result =
          QMessageBoxBuilder( activeWindow()).setTitle( tr("Are you sure you want to quit %1?").arg(applicationName()))
                                             .setText( tr("%1 is about to quit. Tracks played will not be scrobbled if you continue." ).arg(applicationName()) )
                                             .dontAskAgain()
                                             .setIcon( QMessageBox::Question )
                                             .setButtons( QMessageBox::Yes | QMessageBox::No )
                                             .exec(&dontAsk);
    if( result == QMessageBox::Yes )
    {
        unicorn::AppSettings().setValue( "quitDontAsk", dontAsk );
        QCoreApplication::quit();
    }

}

void 
Application::actuallyQuit()
{
    QDialog* d = qobject_cast<QDialog*>( sender());
    if( d ) {
        QCheckBox* dontAskCB = d->findChild<QCheckBox*>();
        if( dontAskCB ) {
            unicorn::AppSettings().setValue( "quitDontAsk", ( dontAskCB->checkState() == Qt::Checked ));
        }
    }
    QCoreApplication::quit();
}

