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
#include <QNetworkProxy>

#include <lastfm/UrlBuilder.h>
#include <lastfm/InternetConnectionMonitor.h>
#include <lastfm/XmlQuery.h>
#include <lastfm/misc.h>
#include <lastfm/NetworkAccessManager.h>

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
#include "lib/unicorn/dialogs/UserManagerDialog.h"
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

#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
#include "Mpris2/Mpris2.h"
#endif

#ifdef Q_OS_WIN32
#include "windows.h"
#endif

using audioscrobbler::Application;

#define ELLIPSIS QString::fromUtf8("…")
#define CONTROL_KEY_CHAR QString::fromUtf8("⌃")
#define APPLE_KEY_CHAR QString::fromUtf8("⌘")
#define SKIP_LIMIT 6

#ifdef Q_WS_X11
#define AS_TRAY_ICON ":/22x22.png"
#define AS_TRAY_ICON_OFF ":/lastfm_icon_22_grayscale.png"
#elif defined( Q_WS_WIN )
#define AS_TRAY_ICON ":/16x16.png"
#define AS_TRAY_ICON_OFF ":/lastfm_icon_16_grayscale.png"
#elif defined( Q_WS_MAC )
#define AS_TRAY_ICON ":/systray_icon_rest_mac.png"
#define AS_TRAY_ICON_OFF ":/mac_control_bar_as_OFF.png"
#endif

Application::Application(int& argc, char** argv) 
    :unicorn::Application( "fm.last.Scrobbler", argc, argv )
    , m_raiseHotKeyId( (void*)-1 )
    , m_reauthenticating( false )
{
    setAttribute( Qt::AA_DontShowIconsInMenus );

    unicorn::AppSettings appSettings;
    int proxyType = appSettings.value( "proxyType", 0 ).toInt();
    QString proxyHost = appSettings.value( "proxyHost", "" ).toString();
    QString proxyPort = appSettings.value( "proxyPort", "" ).toString();
    QString proxyUsername = appSettings.value( "proxyUsername", "" ).toString();
    QString proxyPassword = appSettings.value( "proxyPassword", "" ).toString();

    // set this new proxy
    QNetworkProxy::ProxyType type = QNetworkProxy::DefaultProxy;

    if ( proxyType == 1 )
        type = QNetworkProxy::NoProxy;
    else if ( proxyType == 2 )
        type = QNetworkProxy::HttpProxy;
    else if ( proxyType == 3 )
        type = QNetworkProxy::Socks5Proxy;

    QNetworkProxy proxy( type, proxyHost, proxyPort.toInt(), proxyUsername, proxyPassword );
    lastfm::NetworkAccessManager* nam = qobject_cast<lastfm::NetworkAccessManager*>( lastfm::nam() );

    if ( nam )
        nam->setUserProxy( proxy );

    AudioscrobblerSettings settings;
    lastfm::ws::setScheme( settings.value( "enableSsl", false ).toBool() ? lastfm::ws::Https : lastfm::ws::Http );
}

void
Application::initiateLogin( bool forceWizard ) throw( StubbornUserException )
{
    closeAllWindows();

    if( forceWizard || !unicorn::Settings().firstRunWizardCompleted() )
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

    if ( !currentSession().isValid() )
    {
        // there won't be a current session if one wasn't created by the wizard

        QMap<QString, QString> lastSession = unicorn::Session::lastSessionData();
        if ( lastSession.contains( "username" ) && lastSession.contains( "sessionKey" ) )
            changeSession( lastSession[ "username" ], lastSession[ "sessionKey" ] );
    }

    initiateLogin( !currentSession().isValid() );

    onSessionChanged( currentSession() );

//    QNetworkDiskCache* diskCache = new QNetworkDiskCache(this);
//    diskCache->setCacheDirectory( lastfm::dir::cache().path() );
//    lastfm::nam()->setCache( diskCache );

/// tray
    tray(); // this will initialise m_tray if it doesn't already exist

    /// tray menu
    QMenu* menu = new QMenu;
    m_tray->setContextMenu(menu);

    menu->addMenu( new UserMenu() )->setText( tr( "Accounts" ) );

    m_show_window_action = menu->addAction( tr("Show Scrobbler"));
    m_show_window_action->setShortcut( Qt::CTRL + Qt::META + Qt::Key_S );
    menu->addSeparator();

    {
        m_love_action = new QAction( tr("Love"), this );
        m_love_action->setIconVisibleInMenu( false );
        m_love_action->setCheckable( true );
#ifdef Q_OS_WIN
        QIcon loveIcon;
        loveIcon.addFile( ":/controls_love_OFF_REST.png", QSize( 16, 16 ), QIcon::Normal, QIcon::Off );
        loveIcon.addFile( ":/controls_love_ON_REST.png", QSize( 16, 16 ), QIcon::Normal, QIcon::On );
        m_love_action->setIcon( loveIcon );
#endif
        m_love_action->setEnabled( false );
        connect( m_love_action, SIGNAL(triggered(bool)), SLOT(changeLovedState(bool)));
    }
    {
        m_play_action = new QAction( tr( "Play" ), this );
        m_play_action->setIconVisibleInMenu( false );
        m_play_action->setCheckable( true );
#ifdef Q_OS_WIN
        QIcon playIcon;
        playIcon.addFile( ":/controls_pause_REST.png", QSize(), QIcon::Normal, QIcon::On );
        playIcon.addFile( ":/controls_play_REST.png", QSize(), QIcon::Normal, QIcon::Off );
        m_play_action->setIcon( playIcon );
#endif
    }
    {
        m_skip_action = new QAction( tr( "Skip" ), this );
        m_skip_action->setIconVisibleInMenu( false );
#ifdef Q_OS_WIN
        m_skip_action->setIcon( QIcon( ":/controls_skip_REST.png" ) );
#endif
        m_skip_action->setEnabled( false );
        connect( m_skip_action, SIGNAL(triggered()), SLOT(onSkipTriggered()));
    }
    {
        m_tag_action = new QAction( tr( "Tag" ) + ELLIPSIS, this );
        m_tag_action->setIconVisibleInMenu( false );
#ifdef Q_OS_WIN
        m_tag_action->setIcon( QIcon( ":/controls_tag_REST.png" ) );
#endif
        m_tag_action->setEnabled( false );
        connect( m_tag_action, SIGNAL(triggered()), SLOT(onTagTriggered()));
    }
    {
        m_share_action = new QAction( tr( "Share" ) + ELLIPSIS, this );
        m_share_action->setIconVisibleInMenu( false );
#ifdef Q_OS_WIN
        m_share_action->setIcon( QIcon( ":/controls_share_REST.png" ) );
#endif
        m_share_action->setEnabled( false );
        connect( m_share_action, SIGNAL(triggered()), SLOT(onShareTriggered()));
    }
    {
        m_ban_action = new QAction( tr( "Ban" ), this );
        m_ban_action->setIconVisibleInMenu( false );
#ifdef Q_OS_WIN
        m_ban_action->setIcon( QIcon( ":/controls_ban_REST.png" ) );
#endif
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

    m_mediaKey = new MediaKey( this );
#endif

#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
    new Mpris2( this );
#endif
}

QWidget*
Application::mainWindow() const
{
    return m_mw;
}

QSystemTrayIcon*
Application::tray()
{
    if ( !m_tray )
    {
        m_tray = new QSystemTrayIcon(this);
        setTrayIcon();

#if defined(Q_OS_WIN) || defined(Q_WS_X11)
        connect( m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT( onTrayActivated(QSystemTrayIcon::ActivationReason)) );
#endif
        showAs( unicorn::Settings().showAS() );
        connect( this, SIGNAL( aboutToQuit()), m_tray, SLOT( hide()));
    }

    return m_tray;
}

void
Application::setTrayIcon()
{
    if ( m_tray )
    {
        bool scrobblingOn = unicorn::UserSettings().value( "scrobblingOn", true ).toBool();

        QIcon trayIcon( scrobblingOn ? AS_TRAY_ICON : AS_TRAY_ICON_OFF );
#ifdef Q_WS_MAC
        trayIcon.addFile( ":systray_icon_pressed_mac.png", QSize(), QIcon::Selected );
#endif

        m_tray->setIcon(trayIcon);
    }
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
Application::setBetaUpdates( bool betaUpdates )
{
    m_mw->setBetaUpdates( betaUpdates );
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
             && unicorn::Settings().notifications() )
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
        QFileInfo trackFileInfo( track.url().toLocalFile() );

        if ( trackFileInfo.exists()
             && trackFileInfo.isWritable() ) // this stops us fingerprinting CDs (but maybe other things)
        {
            Fingerprinter* fingerprinter = new Fingerprinter( track, this );
            connect( fingerprinter, SIGNAL(finished()), fingerprinter, SLOT(deleteLater()) );
            fingerprinter->start();
        }
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

void
Application::setMediaKeysEnabled( bool enabled )
{
    m_mediaKey->setEnabled( enabled );
}

#endif

void
Application::onSessionChanged( unicorn::Session& session )
{
    unicorn::UserSettings us( session.user().name() );
    QByteArray skipsData = us.value( "skips" ).toByteArray();
    QDataStream skipsDataStream( &skipsData, QIODevice::ReadWrite );

    skipsDataStream >> m_skips;

    qDebug() << m_skips;
}

void
Application::saveSkips() const
{
    QByteArray skipsData;
    QDataStream skipsDataStream( &skipsData, QIODevice::ReadWrite );
    skipsDataStream << m_skips;

    unicorn::UserSettings us;
    us.setValue( "skips", skipsData );

    qDebug() << m_skips;
}

void
Application::onTrackSpooled( const Track& /*track*/ )
{
}

void
Application::onTrackPaused( bool )
{
}

int
Application::minutesUntilNextSkip( const lastfm::RadioStation& station )
{
    // the next skip can happen 10 minutes after the last skip
    // or when the earliest skip becomes more than an hour old

    int secondsSinceLastSkip = m_skips[ station.url() ].last().secsTo( QDateTime::currentDateTimeUtc() );
    int secondsUntilFirstSkipExpires = (60 * 60) - (m_skips[ station.url() ].first().secsTo( QDateTime::currentDateTimeUtc() ));
    secondsSinceLastSkip = secondsSinceLastSkip == 0 ? 1 : secondsSinceLastSkip; // this stops us saying that there's 11 minutes until the next skip
    return 1 + (qMin( (10 * 60) - secondsSinceLastSkip, secondsUntilFirstSkipExpires ) / 60);
}

void
Application::onSkipTriggered()
{
    QString station = RadioService::instance().station().url();

    bool ok = false;
    int envSkipLimit = QString( qgetenv( "LASTFM_SKIP_LIMIT" ) ).toInt( &ok );
    int skipLimit = ok ? envSkipLimit : SKIP_LIMIT;

    // remove skips for this station that are older than an hour
    while ( (m_skips[ station ].count()
            && m_skips[ station ].head().secsTo( QDateTime::currentDateTimeUtc() ) >= 60 * 60 ) // limit to skips in the last hour
            || m_skips[ station ].count() > skipLimit ) // limit to the last skipLimit skips
        m_skips[ station ].dequeue();

    if ( m_skips[ station ].count() == skipLimit
         && m_skips[ station ].last().secsTo( QDateTime::currentDateTimeUtc() ) < 10 * 60 )
    {
        // There have been skipLimit skips in the last hour
        // and the last skip was under 10 minutes ago
        m_mw->showMessage( tr( "You've reached this station's skip limit. Skip again in %n minute(s).", "", minutesUntilNextSkip( RadioService::instance().station() ) ), "skips", 10 );
    }
    else
    {
        // Make a note of the station and the time that it was skipped
        m_skips[ station ].enqueue( QDateTime::currentDateTimeUtc() );

        if ( m_skips[ station ].count() >= skipLimit - 2 )
        {
            // show a warning that there are only a few skips left
            int skipsLeft = skipLimit - m_skips[ station ].count();

            // if skips is 0 and we got here it's because there were no skips in the last 10 minutes
            if ( skipsLeft <= 0 || m_skips[ station ].last().secsTo( QDateTime::currentDateTimeUtc() ) >= 10 * 60 )
                m_mw->showMessage( tr( "You've reached this station's skip limit. Skip again in %n minute(s).", "", minutesUntilNextSkip( RadioService::instance().station() ) ), "skips", 10 );
            else
                m_mw->showMessage( tr( "You have %n skip(s) remaining on this station.", "", skipsLeft ), "skips", 10 );
        }

        // propagate the skip to the DO skipper
        emit skipTriggered();
    }

    saveSkips();
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

    setTrayIcon();

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

        if ( !m_reauthenticating )
        {
            m_reauthenticating = true;

            QString unauthedUser = aApp->currentSession().user().name();

            unicorn::Settings us;
            us.beginGroup( "Users" );
            us.remove( unauthedUser );
            us.endGroup();

            us.setFirstRunWizardCompleted( false );

            // Tell them that there was an error
            QMessageBoxBuilder( m_mw )
                    .setIcon( QMessageBox::Critical )
                    .setTitle( tr("Authentication Required") )
                    .setText( tr( "<p>The user account <strong>%1</strong> is no longer authenticated with Last.fm.</p>"
                                  "<p>Click OK to start the setup process and reauthenticate this account.</p>" ).arg( unauthedUser ) )
                    .setButtons( QMessageBox::Ok )
                    .exec();


            qobject_cast<unicorn::Application*>( qApp )->restart();


            m_reauthenticating = false;
        }

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

