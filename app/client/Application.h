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

#ifndef AUDIOSCROBBLER_APPLICATION_H_
#define AUDIOSCROBBLER_APPLICATION_H_

#include <QPointer>
#include <QQueue>
#include <QSystemTrayIcon>

#include <lastfm/global.h>
#include <lastfm/Track.h>
#include <lastfm/ws.h>

#include "lib/unicorn/UnicornApplication.h"

#include "Bootstrapper/iTunesBootstrapper.h"
#include "Bootstrapper/PluginBootstrapper.h"

class AboutDialog;
class LicensesDialog;
class MainWindow;
class RadioWidget;
class QAction;
class ScrobbleInfoFetcher;
class Drawer;
class QMenuBar;
class UserManagerDialog;
#ifdef Q_OS_MAC
class MediaKey;
#endif

namespace lastfm { class RadioStation; }
namespace unicorn { class Notify; }
using unicorn::Notify;

#ifdef Q_WS_X11
    class IpodDeviceLinux;
#endif

#if defined(aApp)
#undef aApp
#endif
#define aApp (static_cast<audioscrobbler::Application*>(QCoreApplication::instance()))

namespace audioscrobbler
{
    
    /**
      * @brief Main application logic for the audioscrobbler app.
      *
      * This class contains the core components of the application
      * (ie Audioscrobbler, PlayerConnection etc), top-level gui widgets and the system tray.
      */
    class Application : public unicorn::Application
    {
        Q_OBJECT

        enum Argument
        {
            LastFmUrl,
            Pause, //toggles pause
            Skip,
            Exit,
            Stop,
            Twiddly,
            Settings,
            ArgUnknown
        };

        QMap<QString, QQueue<QDateTime> > m_skips;

        // we delete these so QPointers
        QPointer<QSystemTrayIcon> m_tray;
        QPointer<MainWindow> m_mw;
        QPointer<QMenuBar> m_menuBar;
        QPointer<Notify> m_notify;
        QPointer<UserManagerDialog> m_userManager;
#ifdef Q_OS_MAC
        QPointer<MediaKey> m_mediaKey;
#endif

        QPointer<AbstractBootstrapper> m_bootstrapper;

        Track m_currentTrack;
        Track m_trackToScrobble;

        void* m_raiseHotKeyId;

        QPointer<AboutDialog> m_aboutDialog;
        QPointer<LicensesDialog> m_licensesDialog;
        
        QAction* m_submit_scrobbles_toggle;
        QAction* m_love_action;
        QAction* m_tag_action;
        QAction* m_share_action;
        QAction* m_ban_action;
        QAction* m_play_action;
        QAction* m_skip_action;
        QAction* m_show_window_action;
        QAction* m_toggle_window_action;
        QAction* m_scrobble_ipod_action;
        QAction* m_visit_profile_action;
        QAction* m_mute_action;

        bool m_reauthenticating;
        
    public:
        Application(int& argc, char** argv);

        void init();
        
        QAction* loveAction() const { return m_love_action; }
        QAction* tagAction() const { return m_tag_action; }
        QAction* shareAction() const { return m_share_action; }
        QAction* banAction() const { return m_ban_action; }
        QAction* playAction() const { return m_play_action; }
        QAction* skipAction() const { return m_skip_action; }
        QAction* muteAction() const { return m_mute_action; }
        QAction* scrobbleToggleAction() const { return m_submit_scrobbles_toggle; }
        QSystemTrayIcon* tray();

        QWidget* mainWindow() const;

        void setBetaUpdates( bool betaUpdates );

        void setRaiseHotKey( Qt::KeyboardModifiers mods, int key );

        void startBootstrap( const QString& pluginId );

        void showAs( bool showAs );

#ifdef Q_OS_MAC
        void setMediaKeysEnabled( bool enabled );
#endif
        QString currentCategory() const;
        
    signals:
        void lovedStateChanged(bool loved);

        // re-route all the info fetchers singals
        void trackGotInfo(const XmlQuery& lfm);
        void albumGotInfo(const XmlQuery& lfm);
        void artistGotInfo(const XmlQuery& lfm);
        void artistGotEvents(const XmlQuery& lfm);
        void trackGotTopFans(const XmlQuery& lfm);
        void trackGotTags(const XmlQuery& lfm);

        void finished();

        void skipTriggered();
		
        void error( const QString& message );
        void status( const QString& message, const QString& id );
        void showMessage( const QString& message, const QString& id );

        void bootstrapStarted( const QString& pluginId );
        void bootstrapDone( int status );

        void scrobbleToggled( bool on );

    public slots:
        void showWindow();
        void quit();
        void actuallyQuit();

        void changeLovedState(bool loved);
        void onBusLovedStateChanged(bool);

        void onTrackGotInfo(const XmlQuery& );
        void parseArguments( const QStringList& args );

        void onPrefsTriggered();
        void onDiagnosticsTriggered();

        void onScrobbleToggled( bool scrobblingOn );

    protected:
        virtual void initiateLogin( bool forceWizard ) throw( StubbornUserException );

    private:
        static Argument argument( const QString& arg );
        void saveSkips() const;
        int minutesUntilNextSkip( const lastfm::RadioStation& station );

#ifdef Q_OS_MAC
        bool macEventFilter ( EventHandlerCallRef caller, EventRef event );
#endif

    private slots:
        void onTrayActivated(QSystemTrayIcon::ActivationReason);
        void onCorrected(QString correction);

        void onSkipTriggered();
        void onTagTriggered();
        void onShareTriggered();

        void onVisitProfileTriggered();
        void onFaqTriggered();
        void onForumsTriggered();
        void onAboutTriggered();
        void onTourTriggered();
        void onLicensesTriggered();

        void toggleWindow();

        void onTrackStarted( const Track&, const Track& );
        void onTrackPaused( bool );

        void onTrackSpooled( const Track& );

        void onSessionChanged( unicorn::Session& );

        void onMessageReceived(const QStringList& message);

        void setTrayIcon();
		
        /** all webservices connect to this and emit in the case of bad errors that
	     * need to be handled at a higher level */
        void onWsError( lastfm::ws::Error );
    };
}

#endif //AUDIOSCROBBER_APPLICATION_H_
