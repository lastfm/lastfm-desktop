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

#include "UnicornSettings.h"
#include "UnicornApplication.h"
#include <lastfm/User.h>

unicorn::Settings::Settings()
    :QSettings( unicorn::organizationName(), "" )
{}

QList<lastfm::User>
unicorn::Settings::userRoster() const
{
    const_cast<Settings*>(this)->beginGroup( "Users" );
    QList<User> ret;
    foreach( QString child, childGroups()) {
        if( child == "com" || !contains( child + "/SessionKey" )) continue;
        ret << User( child );
    }
    const_cast<Settings*>(this)->endGroup();
    return ret;
}


bool
unicorn::Settings::firstRunWizardCompleted() const
{
    return value( "FirstRunWizardCompletedBeta", false ).toBool();
}

void
unicorn::Settings::setFirstRunWizardCompleted( bool firstRunWizardCompleted )
{
    setValue( "FirstRunWizardCompletedBeta", firstRunWizardCompleted );
}

bool
unicorn::Settings::betaUpdates() const
{
    return value( "betaUpdates", false ).toBool();
}

void
unicorn::Settings::setBetaUpdates( bool betaUpdates )
{
    setValue( "betaUpdates", betaUpdates );
}

void
unicorn::Settings::showWhere()
{
    int showWhereIndex = value( "showWhere", -1 ).toInt();

    if ( showWhereIndex != -1 )
    {
        // 0 == show both, 1 == show only dock, 2 == show only menu bar
        setShowAS( showWhereIndex != 1 );
        setShowDock( showWhereIndex != 2 );

        remove( "showWhere" ); // don't read this setting again
    }
}

bool
unicorn::Settings::showAS()
{
    showWhere();
    return value( "showAS", true ).toBool();
}

void
unicorn::Settings::setShowAS( bool showAS )
{
    setValue( "showAS", showAS );
}

bool
unicorn::Settings::showDock()
{
    showWhere();
    return value( "showDock", true ).toBool();
}

void
unicorn::Settings::setShowDock( bool showDock )
{
    setValue( "showDock", showDock );
}

bool
unicorn::Settings::notifications() const
{
    return value( "notifications", true ).toBool();
}

void
unicorn::Settings::setNotifications( bool notifications )
{
    setValue( "notifications", notifications );
}

bool
unicorn::Settings::sendCrashReports() const
{
    return value( "sendCrashReports", true ).toBool();
}

void
unicorn::Settings::setSendCrashReports( bool sendCrashReports )
{
    setValue( "sendCrashReports", sendCrashReports );
}

bool
unicorn::Settings::checkForUpdates() const
{
    return value( "checkForUpdates", true ).toBool();
}

void
unicorn::Settings::setCheckForUpdates( bool checkForUpdates )
{
    setValue( "checkForUpdates", checkForUpdates );
}

unicorn::AppSettings::AppSettings( QString appname )
    : QSettings( unicorn::organizationName(), appname.isEmpty() ? qApp->applicationName() : appname )
{}

unicorn::UserSettings::UserSettings( QString username )
{
    beginGroup( "Users" );
    beginGroup( username );
}


bool
unicorn::AppSettings::alwaysAsk() const
{
    return value( "alwaysAsk", true ).toBool();
}

void
unicorn::AppSettings::setAlwaysAsk( bool alwaysAsk )
{
    setValue( "alwaysAsk", alwaysAsk );
}

unicorn::OldeAppSettings::OldeAppSettings()
    : AppSettings(
#ifdef Q_OS_MAC
"scrobbler"
#else
"Client"
#endif
          )
{}

bool
unicorn::OldeAppSettings::deviceScrobblingEnabled() const
{
    return value( "iPodScrobblingEnabled", true ).toBool();
}

void
unicorn::OldeAppSettings::setDeviceScrobblingEnabled( bool deviceScrobblingEnabled )
{
    setValue( "iPodScrobblingEnabled", deviceScrobblingEnabled );
}

bool
unicorn::OldeAppSettings::launchWithMediaPlayers() const
{
    return value( "LaunchWithMediaPlayer", true ).toBool();
}

void
unicorn::OldeAppSettings::setLaunchWithMediaPlayers( bool LaunchWithMediaPlayers )
{
    setValue( "LaunchWithMediaPlayer", LaunchWithMediaPlayers );
}

unicorn::Session::Info
unicorn::UserSettings::sessionInfo()
{
    unicorn::Session::Info info;

    beginGroup( "Session" );

    info.valid = value( "valid", false ).toBool();
    info.subscriptionPrice = value( "subscriptionPrice", "" ).toString();
    info.youRadio = value( "youRadio", false ).toBool();
    info.registeredRadio = value( "registeredRadio", false ).toBool();
    info.subscriberRadio = value( "subscriberRadio", false ).toBool();
    info.youWebRadio = value( "youWebRadio", false ).toBool();
    info.registeredWebRadio = value( "registeredWebRadio", false ).toBool();
    info.subscriberWebRadio = value( "subscriberWebRadio", false ).toBool();

    endGroup();

    return info;
}

void
unicorn::UserSettings::setSessionInfo( const unicorn::Session::Info& sessionInfo )
{
    beginGroup( "Session" );
    setValue( "valid", sessionInfo.valid );
    setValue( "subscriptionPrice", sessionInfo.subscriptionPrice );
    setValue( "youRadio", sessionInfo.youRadio );
    setValue( "registeredRadio", sessionInfo.registeredRadio );
    setValue( "subscriberRadio", sessionInfo.subscriberRadio );
    setValue( "youWebRadio", sessionInfo.youWebRadio );
    setValue( "registeredWebRadio", sessionInfo.registeredWebRadio );
    setValue( "subscriberWebRadio", sessionInfo.subscriberWebRadio );
    endGroup();
}

bool
unicorn::UserSettings::subscriber() const
{
    return value( "subscriber", false ).toBool();
}

void
unicorn::UserSettings::setSubscriber( bool subscriber )
{
    setValue( "subscriber", subscriber );
}

QString
unicorn::UserSettings::sessionKey() const
{
    return value( "SessionKey", "" ).toString();
}

void
unicorn::UserSettings::setSessionKey( const QString& sessionKey )
{
    setValue( "SessionKey", sessionKey );
}

quint32
unicorn::UserSettings::scrobbleCount() const
{
    return value( "ScrobbleCount", 0 ).toUInt();
}

void
unicorn::UserSettings::setScrobbleCount( quint32 scrobbleCount )
{
    setValue( "ScrobbleCount", scrobbleCount );
}

QDateTime
unicorn::UserSettings::dateRegistered() const
{
    return value( "DateRegistered", false ).toDateTime();
}

void
unicorn::UserSettings::setDateRegistered( const QDateTime& dateRegistered )
{
    setValue( "DateRegistered", dateRegistered );
}

QString
unicorn::UserSettings::realName() const
{
    return value( "RealName", "" ).toString();
}

void
unicorn::UserSettings::setRealName( const QString& realName )
{
    setValue( "RealName", realName );
}

User::Type
unicorn::UserSettings::type() const
{
    return static_cast<User::Type>( value( "Type", User::TypeUser ).toInt() );
}

void
unicorn::UserSettings::setType( User::Type type )
{
    setValue( "Type", type );
}

double
unicorn::UserSettings::scrobblePoint() const
{
    return value( "scrobblePoint", 50 ).toDouble();
}

void
unicorn::UserSettings::setScrobblePoint( double scrobblePoint )
{
    setValue( "scrobblePoint", scrobblePoint );
}

bool
unicorn::UserSettings::fingerprinting() const
{
    return value( "fingerprint", true ).toBool();
}

void
unicorn::UserSettings::setFingerprinting( bool fingerprinting )
{
    setValue( "fingerprint", fingerprinting );
}

bool
unicorn::UserSettings::podcasts() const
{
    return value( "podcasts", true ).toBool();
}

void
unicorn::UserSettings::setPodcasts( bool podcasts )
{
    setValue( "podcasts", podcasts );
}

bool
unicorn::UserSettings::scrobblingOn() const
{
    return value( "scrobblingOn", true ).toBool();
}

void
unicorn::UserSettings::setScrobblingOn( bool scrobblingOn )
{
    setValue( "scrobblingOn", scrobblingOn );
}

QStringList
unicorn::UserSettings::exclusionDirs() const
{
    return value( "ExclusionDirs", QStringList() ).toStringList();
}

void
unicorn::UserSettings::setExclusionDirs( const QStringList& exclusionDirs )
{
    setValue( "ExclusionDirs", exclusionDirs );
}

bool
unicorn::UserSettings::enforceScrobbleTimeMax() const
{
    return value( "enforceScrobbleTimeMax", true ).toBool();
}

void
unicorn::UserSettings::setEnforceScrobbleTimeMax( bool enforceScrobbleTimeMax )
{
    setValue( "enforceScrobbleTimeMax", enforceScrobbleTimeMax );
}
