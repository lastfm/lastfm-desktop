/*
   Copyright 2012 Last.fm Ltd.
      - Primarily authored by Michael Coffey

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

#include <QTimer>
#include <QPushButton>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

#ifndef Q_OS_MAC
#include "../plugins/ITunesPluginInfo.h"
#endif

#include "CloseAppsDialog.h"
#include "ui_CloseAppsDialog.h"


unicorn::CloseAppsDialog::CloseAppsDialog( const QList<IPluginInfo*>& plugins, QWidget *parent )
    :QDialog( parent ),
      ui(new Ui::CloseAppsDialog),
      m_plugins( plugins ),
      m_ownsPlugins( false )
{
    commonSetup();
}

unicorn::CloseAppsDialog::CloseAppsDialog(QWidget *parent) :
    QDialog( parent ),
    ui(new Ui::CloseAppsDialog)
{
    commonSetup();
}

void
unicorn::CloseAppsDialog::setTitle( const QString& title )
{
    setWindowTitle( title );
}

void
unicorn::CloseAppsDialog::setDescription( const QString& description )
{
    ui->text->setText( description );
}

void
unicorn::CloseAppsDialog::showPluginList( bool showPluginList )
{
    ui->listWidget->setVisible( showPluginList );
}

void
unicorn::CloseAppsDialog::commonSetup()
{
    ui->setupUi(this);

    ui->text->setText( tr( "Please close the following apps to continue." ) );

    if ( runningApps().count() == 0 )
        hide();

    checkApps();

    QTimer* timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(checkApps()) );
    timer->setInterval( 1000 );
    timer->start();
}

void
unicorn::CloseAppsDialog::setOwnsPlugins( bool ownsPlugins )
{
    m_ownsPlugins = ownsPlugins;
}

bool
unicorn::CloseAppsDialog::isITunesRunning()
{
    QStringList apps;
#ifndef Q_OS_MAC
    QList<IPluginInfo*> plugins;
    ITunesPluginInfo* iTunesPluginInfo = new ITunesPluginInfo;
    plugins << iTunesPluginInfo;
    apps = runningApps( plugins );
    delete iTunesPluginInfo;
#else
    apps = runningApps();
#endif
    return apps.count() == 1;
}

void
unicorn::CloseAppsDialog::checkApps()
{
    QStringList apps = runningApps();

    ui->listWidget->setUpdatesEnabled( false );

    ui->listWidget->clear();

    foreach ( const QString& app, apps )
        new QListWidgetItem( app, ui->listWidget );

    ui->listWidget->setUpdatesEnabled( true );

    if ( apps.count() == 0 )
        accept();
}

void
unicorn::CloseAppsDialog::showEvent( QShowEvent* event )
{
    adjustSize();
}

#ifndef Q_OS_MAC
QStringList
unicorn::CloseAppsDialog::runningApps()
{
    return runningApps( m_plugins );
}

QStringList
unicorn::CloseAppsDialog::runningApps( const QList<IPluginInfo*>& plugins )
{
    QStringList apps;

    DWORD aProcesses[1024];
    DWORD cbNeeded;
    DWORD cProcesses;
    unsigned int i;

    if ( EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        // Calculate how many process identifiers were returned.

        cProcesses = cbNeeded / sizeof(DWORD);

        // Print the name and process identifier for each process.

        for ( i = 0; i < cProcesses; ++i )
        {
            if( aProcesses[i] != 0 )
            {
                TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

                // Get a handle to the process.
                HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                PROCESS_VM_READ,
                FALSE, aProcesses[i] );

                if (NULL != hProcess )
                {
                    HMODULE hMod;
                    DWORD cbNeeded;

                    if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
                    {
                        GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );

                        foreach( IPluginInfo* plugin, plugins )
                            if ( plugin->processName().compare( QString::fromStdWString( szProcessName ), Qt::CaseInsensitive ) == 0 )
                                apps << plugin->name();
                    }
                }

                // Release the handle to the process.
                CloseHandle( hProcess );
            }
        }
    }

    return apps;
}
#endif

unicorn::CloseAppsDialog::~CloseAppsDialog()
{
    delete ui;
#ifdef Q_OS_WIN
    if ( m_ownsPlugins )
        foreach ( IPluginInfo* plugin, m_plugins )
            delete plugin;
#endif
}
