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
#ifndef UNICORN_SETTINGS_H
#define UNICORN_SETTINGS_H

#include "lib/unicorn/UnicornSession.h"

#include "lib/DllExportMacro.h"

#include <lastfm/User.h>

#include <QSettings>
#include <QString>
#include <QCoreApplication>

namespace unicorn
{
    inline const char* organizationName() { return "Last.fm"; }
    inline const char* organizationDomain() { return "last.fm"; }

    class UNICORN_DLLEXPORT Settings : public QSettings
    {
    public:
        Settings();

        QList<lastfm::User> userRoster() const;

        bool firstRunWizardCompleted() const;
        void setFirstRunWizardCompleted( bool firstRunWizardCompleted );

        bool betaUpdates() const;
        void setBetaUpdates( bool betaUpdates );

        bool showAS();
        void setShowAS( bool showAS );

        bool showDock();
        void setShowDock( bool showDock );

        bool notifications() const;
        void setNotifications( bool notifications );

        bool sendCrashReports() const;
        void setSendCrashReports( bool sendCrashReports );

        bool checkForUpdates() const;
        void setCheckForUpdates( bool checkForUpdates );


    private:
        void showWhere();
    };

    class UNICORN_DLLEXPORT AppSettings : public QSettings
    {
    public:
        AppSettings( QString appname = QCoreApplication::applicationName() );

        bool alwaysAsk() const;
        void setAlwaysAsk( bool alwaysAsk );
    };

    class UNICORN_DLLEXPORT OldeAppSettings : public AppSettings
    {
    public:
        OldeAppSettings();

        bool deviceScrobblingEnabled() const;
        void setDeviceScrobblingEnabled( bool deviceScrobblingEnabled );

        bool launchWithMediaPlayers() const;
        void setLaunchWithMediaPlayers( bool launchWithMediaPlayers );
    };


    /** Clearly no use until a username() has been assigned. But this is
      * automatic if you use unicorn::Application anyway. */
    class UNICORN_DLLEXPORT UserSettings : public Settings
    {
    public:
        struct SessionInfo
        {

        };

        UserSettings( QString userName = User() );

        unicorn::Session::Info sessionInfo();
        void setSessionInfo( const unicorn::Session::Info& sessionInfo );

        bool subscriber() const;
        void setSubscriber( bool subscriber );

        QString sessionKey() const;
        void setSessionKey( const QString& sessionKey );

        quint32 scrobbleCount() const;
        void setScrobbleCount( quint32 scrobbleCount );

        QDateTime dateRegistered() const;
        void setDateRegistered( const QDateTime& dateRegistered );

        QString realName() const;
        void setRealName( const QString& realName );

        User::Type type() const;
        void setType( User::Type type );

        double scrobblePoint() const;
        void setScrobblePoint( double scrobblePoint );

        bool fingerprinting() const;
        void setFingerprinting( bool fingerprinting );

        bool podcasts() const;
        void setPodcasts( bool podcasts );

        bool scrobblingOn() const;
        void setScrobblingOn( bool scrobblingOn );

        QStringList exclusionDirs() const;
        void setExclusionDirs( const QStringList& exclusionDirs );

        bool enforceScrobbleTimeMax() const;
        void setEnforceScrobbleTimeMax( bool enforceScrobbleTimeMax );
    };
}


#endif
