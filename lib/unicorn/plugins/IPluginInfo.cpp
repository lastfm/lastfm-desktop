/*
   Copyright 2010-2013 Last.fm Ltd.
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

#include <QSettings>
#include <QStringList>
#include <QCoreApplication>
#include <QProcess>
#include <QDebug>

#include <windows.h>
#include <Shellapi.h>
#include <Wtsapi32.h>

#include "lib/unicorn/QMessageBoxBuilder.h"

#include "lib/unicorn/UnicornSettings.h"

#include "../Dialogs/CloseAppsDialog.h"

#include "IPluginInfo.h"
#include "KillProcess.h"

unicorn::IPluginInfo::IPluginInfo( QObject* parent )
    :QObject( parent )
    , m_install( false )
    , m_verbose( false )
{}

bool
unicorn::IPluginInfo::doInstall()
{
    bool success = false;

    QList<IPluginInfo*> plugins;
    plugins << this;

    CloseAppsDialog* closeApps = new CloseAppsDialog( plugins, 0 );

    if ( closeApps->result() != QDialog::Accepted )
        closeApps->exec();
    else
        closeApps->deleteLater();

    if ( closeApps->result() == QDialog::Accepted )
    {
        QString installer = QString( "\"%1\"" ).arg( QCoreApplication::applicationDirPath() + "/plugins/" + pluginInstaller() );
        qDebug() << installer;
        QProcess* installerProcess = new QProcess( this );
        QStringList args;
        if ( !m_verbose )
            args << "/SILENT";
        installerProcess->start( installer, args );
        success = installerProcess->waitForFinished( -1 );

        if ( m_verbose )
        {
            if ( !success )
            {
                // Tell the user that
                QMessageBoxBuilder( 0 ).setTitle( tr( "Plugin install error" ) )
                        .setIcon( QMessageBox::Information )
                        .setText( tr( "<p>There was an error updating your plugin.</p>"
                                      "<p>Please try again later.</p>" ) )
                        .setButtons( QMessageBox::Ok )
                        .exec();
            }
            else
            {
                // The user didn't closed their media players
                QMessageBoxBuilder( 0 ).setTitle( tr( "Plugin installed!" ) )
                        .setIcon( QMessageBox::Information )
                        .setText( tr( "<p>The %1 plugin has been installed.</p>"
                                      "<p>You're now ready to scrobble with %1.</p>" ).arg( name() ) )
                        .setButtons( QMessageBox::Ok )
                        .exec();
            }
        }
    }
    else
    {
        // The user didn't close their media players
        QMessageBoxBuilder( 0 ).setTitle( tr( "The %1 plugin hasn't been installed" ).arg( name() ) )
                .setIcon( QMessageBox::Warning )
                .setText( tr( "You didn't close %1 so its plugin hasn't been installed." ).arg( name() ) )
                .setButtons( QMessageBox::Ok )
                .exec();
    }

    return success;
}

bool
unicorn::IPluginInfo::install() const
{
    return m_install;
}


void
unicorn::IPluginInfo::install( bool install )
{
    m_install = install;
}

#ifdef Q_OS_WIN
BOOL
unicorn::IPluginInfo::isWow64()
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    void* fnIsWow64Process = GetProcAddress(
    GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!IsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}
#endif


bool
unicorn::IPluginInfo::isInstalled() const
{
    QSettings settings( "HKEY_LOCAL_MACHINE\\SOFTWARE\\Last.fm\\Client\\Plugins", QSettings::NativeFormat );
    return settings.childGroups().contains( id() );
}

unicorn::Version
unicorn::IPluginInfo::installedVersion() const
{
    QSettings settings( "HKEY_LOCAL_MACHINE\\SOFTWARE\\Last.fm\\Client\\Plugins\\" + id(), QSettings::NativeFormat );
    return Version::fromString( settings.value( "Version", "0.0.0.0" ).toString() );
}

bool
unicorn::IPluginInfo::canBootstrap() const
{
    return bootstrapType() != NoBootstrap;
}

QString
unicorn::IPluginInfo::programFilesX86() const
{
    return QString( getenv( "ProgramFiles(x86)" ) );
}

QString
unicorn::IPluginInfo::programFiles64() const
{
    return QString( getenv( "ProgramW6432" ) );
}

void
unicorn::IPluginInfo::setVerbose( bool verbose )
{
    m_verbose = verbose;
}

