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
#ifndef UNICORN_APPLICATION_H
#define UNICORN_APPLICATION_H

#include "qtsingleapplication/qtsingleapplication.h"

#include "common/HideStupidWarnings.h"
#include "lib/DllExportMacro.h"
#include "UnicornSession.h"
#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QPointer>

#ifdef Q_OS_MAC
#include "UnicornApplicationDelegate.h"
#endif

#ifdef Q_OS_MAC64
#include <Carbon/Carbon.h>
#endif

#ifdef Q_OS_MAC
#define OLDE_PLUGIN_SETTINGS "scrobbler"
#else
#define OLDE_PLUGIN_SETTINGS "Client"
#endif

#define SETTING_SHOW_AS "showAS"
#define SETTING_LAUNCH_ITUNES "LaunchWithMediaPlayer"
#define SETTING_NOTIFICATIONS "notifications"
#define SETTING_LAST_RADIO "lastRadio"
#define SETTING_SEND_CRASH_REPORTS "sendCrashReports"
#define SETTING_CHECK_UPDATES "checkUpdates"
#define SETTING_BETA_UPGRADES "betaUpdates"
#define SETTING_HIDE_DOCK "hideDock"
#define SETTING_SHOW_WHERE "showWhere"
#define SETTING_OLDE_ITUNES_DEVICE_SCROBBLING_ENABLED "iPodScrobblingEnabled"
#define SETTING_ALWAYS_ASK "alwaysAsk"

#define SETTING_FIRST_RUN_WIZARD_COMPLETED "FirstRunWizardCompletedBeta"

namespace lastfm{
    class User;
    class InternetConnectionMonitor;
}

class LoginContinueDialog;
class QNetworkReply;

namespace unicorn
{
    class Bus;

    /**
     * Unicorn base Application.
     *
     * Child classes should make sure to call the protected function initiateLogin
     * on their constructor otherwise the app would probably crash, as there will be
     * no valid user session.
     */
    class UNICORN_DLLEXPORT Application : public QtSingleApplication
    {
        Q_OBJECT

        bool m_logoutAtQuit;

    public:
        class StubbornUserException
        {};

        /** will put up the log in dialog if necessary, throwing if the user
          * cancels, ie. they refuse to log in */
        Application(const QString &id, int &argc, char **argv) throw( StubbornUserException );
        ~Application();

        virtual void init();

        /** Will return the actual stylesheet that is loaded if one is specified
           (on the command-line with -stylesheet or with setStyleSheet(). )
           Note. the QApplication styleSheet property will return the path 
                 to the css file unlike this method. */
        const QString& loadedStyleSheet() const {
            return m_styleSheet;
        }

        Session& currentSession() const;

        static unicorn::Application* instance(){ return (unicorn::Application*)qApp; }
        void* installHotKey( Qt::KeyboardModifiers, quint32, QObject* receiver, const char* slot );
        void unInstallHotKey( void* id );
        bool isInternetConnectionUp() const;

        void translate();
#ifdef Q_OS_MAC
        void macTranslate( const QString& lang );
        void hideDockIcon( bool hideDockIcon );
        UnicornApplicationDelegate* delegate() const { return m_delegate; }
#endif
    public slots:
        void manageUsers();
        void changeSession( const QString& username, const QString& sessionKey, bool announce = true );
        void sendBusLovedStateChanged(bool loved);
        void refreshStyleSheet();
        void restart();

    private:
        void changeSession( unicorn::Session* newSession, bool announce = true );
        void setupHotKeys();
        void onHotKeyEvent(quint32 id);
        void loadStyleSheet( QFile& );
        QMainWindow* findMainWindow();

        QString m_styleSheet;
        QPointer<Session> m_currentSession;
        bool m_wizardRunning;
        QMap< quint32, QPair<QObject*, const char*> > m_hotKeyMap;
        QString m_cssDir;
        QString m_cssFileName;
#ifdef Q_OS_MAC
        QPointer<UnicornApplicationDelegate> m_delegate;

        void setOpenApplicationEventHandler();
        void setGetURLEventHandler();
    public:
        void appleEventReceived( const QStringList& messages );

    private:
        static OSStatus hotkeyEventHandler( EventHandlerCallRef, EventRef, void* );

#ifdef Q_OS_MAC64
        static OSErr appleEventHandler( const AppleEvent*, AppleEvent*, void* );
#else
        static short appleEventHandler( const AppleEvent*, AppleEvent*, long );
#endif
#endif
#ifdef WIN32
        static bool winEventFilter ( void* );
#endif
    protected:
        /**
         * Reimplement this function if you want to control the initial login process.
         */
        virtual void initiateLogin( bool forceWizard = false ) throw( StubbornUserException );

        void setWizardRunning( bool running );

        QPointer<Bus> m_bus;
        lastfm::InternetConnectionMonitor* m_icm;
	

    private slots:
        void onWizardRunningQuery( const QString& );
        void onBusSessionQuery( const QString& );
        void onBusSessionChanged( const unicorn::Session& session );

    signals:
        void gotUserInfo( const lastfm::User& user );
        void sessionChanged( const unicorn::Session& session );
        void rosterUpdated();
        void busLovedStateChanged(bool loved);
        void internetConnectionUp();
        void internetConnectionDown();
    };
}

#endif
