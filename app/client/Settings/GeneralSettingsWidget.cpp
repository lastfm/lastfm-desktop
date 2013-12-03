/*
   Copyright 2011 Last.fm Ltd.
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

#include <QComboBox>

#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/QMessageBoxBuilder.h"

#include "../Application.h"

#include "ui_GeneralSettingsWidget.h"
#include "GeneralSettingsWidget.h"

GeneralSettingsWidget::GeneralSettingsWidget( QWidget* parent )
    :SettingsWidget( parent ),
      ui( new Ui::GeneralSettingsWidget )
{
    ui->setupUi( this );

    populateLanguages();

    ui->notifications->setChecked( unicorn::Settings().notifications() );
    ui->sendCrashReports->setChecked( unicorn::Settings().sendCrashReports() );
    ui->beta->setChecked( unicorn::Settings().betaUpdates() );

#if !defined( Q_OS_WIN ) && !defined( Q_OS_MAC )
    ui->beta->hide(); // only have a beta update setting in mac and windows
#endif

#ifdef Q_OS_MAC
    ui->mediaKeys->setChecked( unicorn::Settings().value( "mediaKeys", true ).toBool() );

    ui->showAs->setChecked( unicorn::Settings().showAS() );
    ui->showDock->setChecked( unicorn::Settings().showDock() );

#else
    ui->showDock->hide();
    ui->mediaKeys->hide();

    ui->showAs->setChecked( unicorn::Settings().showAS() );
#endif

#ifndef Q_WS_X11
    ui->launch->setChecked( unicorn::OldeAppSettings().launchWithMediaPlayers() );
    ui->updates->setChecked( unicorn::Settings().checkForUpdates() );
#else
    ui->launch->hide();
    ui->updates->hide();
#endif

    connect( ui->languages, SIGNAL( currentIndexChanged( int ) ), SLOT( onSettingsChanged() ) );
    connect( ui->notifications, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
    connect( ui->sendCrashReports, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
    connect( ui->beta, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
    connect( ui->showAs, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
    connect( ui->showDock, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
    connect( ui->mediaKeys, SIGNAL(stateChanged(int)), SLOT(onSettingsChanged()) );    connect( ui->launch, SIGNAL(stateChanged(int) ), SLOT( onSettingsChanged() ) );
    connect( ui->updates, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
}

void
GeneralSettingsWidget::populateLanguages()
{
    ui->languages->addItem( tr( "System Language" ), "" );
    ui->languages->addItem( "English", QLocale( QLocale::English, QLocale::UnitedKingdom ).name() );
    ui->languages->addItem( QString::fromUtf8( "français" ), QLocale( QLocale::French ).name() );
    ui->languages->addItem( "Italiano", QLocale( QLocale::Italian ).name() );
    ui->languages->addItem( "Deutsch", QLocale( QLocale::German ).name() );
    ui->languages->addItem( QString::fromUtf8( "Español" ), QLocale( QLocale::Spanish ).name() );
    ui->languages->addItem( QString::fromUtf8( "Português" ), QLocale( QLocale::Portuguese, QLocale::Brazil ).name() );
    ui->languages->addItem( "Polski", QLocale( QLocale::Polish ).name() );
    ui->languages->addItem( "Svenska", QLocale( QLocale::Swedish ).name());
    ui->languages->addItem( QString::fromUtf8( "Türkçe" ), QLocale( QLocale::Turkish ).name() );
    ui->languages->addItem( QString::fromUtf8( "Pусский" ), QLocale( QLocale::Russian ).name() );
    ui->languages->addItem( QString::fromUtf8( "简体中文" ), QLocale( QLocale::Chinese, QLocale::China ).name() );
    ui->languages->addItem( QString::fromUtf8( "日本語" ), QLocale( QLocale::Japanese ).name());

    QString currLanguage = unicorn::AppSettings().value( "language", "" ).toString();
    int index = ui->languages->findData( currLanguage );
    if ( index != -1 )
    {
        ui->languages->setCurrentIndex( index );
    }
}

void
GeneralSettingsWidget::saveSettings()
{
    qDebug() << "has unsaved changes?" << hasUnsavedChanges();
    if ( hasUnsavedChanges() )
    {
        bool restartNeeded = false;

        int currIndex = ui->languages->currentIndex();
        QString currLanguage = ui->languages->itemData( currIndex ).toString();

        if ( unicorn::AppSettings().value( "language", "" ) != currLanguage )
        {
            if ( currLanguage == ""  )
                QLocale::setDefault( QLocale::system() );
            else
                QLocale::setDefault( QLocale( currLanguage ) );

            unicorn::AppSettings().setValue( "language", currLanguage );

#ifdef Q_OS_MAC
            aApp->translate();
#endif

            restartNeeded = true;
        }

        // setting is for the 'Client' aplication for compatibility with old media player plugins
        unicorn::OldeAppSettings().setLaunchWithMediaPlayers( ui->launch->isChecked() );

        unicorn::Settings().setNotifications( ui->notifications->isChecked() );
        unicorn::Settings().setSendCrashReports( ui->sendCrashReports->isChecked() );
        unicorn::Settings().setCheckForUpdates( ui->updates->isChecked() );
        unicorn::Settings().setBetaUpdates( ui->beta->isChecked() );

        aApp->setBetaUpdates( ui->beta->isChecked() );

#ifdef Q_OS_MAC
        /// media keys
        unicorn::Settings().setValue( "mediaKeys", ui->mediaKeys->isChecked() );
        aApp->setMediaKeysEnabled( ui->mediaKeys->isChecked() );

        /// dock hiding
        bool showDockOld = unicorn::Settings().showDock();
        unicorn::Settings().setShowDock( ui->showDock->isChecked() );

        if ( showDockOld != ui->showDock->isChecked() )
        {
            // the setting has changed
            aApp->showDockIcon( ui->showDock->isChecked() );

            // Hiding the dock icon while the app is running is not supported on Snow Leopard
            if ( QSysInfo::MacintoshVersion <= QSysInfo::MV_10_6 && !ui->showDock->isChecked() )
                restartNeeded = true;
        }
#endif

        unicorn::Settings().setShowAS( ui->showAs->isChecked() );
        aApp->showAs( ui->showAs->isChecked() );

        onSettingsSaved();

        if ( restartNeeded )
        {
            int button = QMessageBoxBuilder( this )
                            .setIcon( QMessageBox::Question )
                            .setTitle( tr( "Restart now?" ) )
                            .setText( tr( "An application restart is required for the change to take effect. Would you like to restart now?" ) )
                            .setButtons( QMessageBox::Yes | QMessageBox::No )
                            .exec();

            if ( button == QMessageBox::Yes )
                aApp->restart();
        }
    }
}
