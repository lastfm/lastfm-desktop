/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Jono Cole, Michael Coffey, and William Viana

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

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QStringListModel>
#include <QNetworkProxy>
#include <QSslSocket>

#include <lastfm/User.h>
#include <lastfm/ws.h>
#include <lastfm/NetworkAccessManager.h>

#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/UnicornApplication.h"
#include "lib/unicorn/UnicornSession.h"
#include "lib/unicorn/widgets/UserManagerWidget.h"

#include "../Widgets/ShortcutEdit.h"
#include "../AudioscrobblerSettings.h"
#include "../Application.h"
#include "ui_AdvancedSettingsWidget.h"
#include "AdvancedSettingsWidget.h"

using lastfm::User;

AdvancedSettingsWidget::AdvancedSettingsWidget( QWidget* parent )
    : SettingsWidget( parent ),
      ui( new Ui::AdvancedSettingsWidget )
{
    ui->setupUi( this );

    AudioscrobblerSettings settings;

#ifdef Q_WS_X11
    ui->shortcuts->hide();
#else

    ui->sce->setTextValue( settings.raiseShortcutDescription() );
    ui->sce->setModifiers( settings.raiseShortcutModifiers() );
    ui->sce->setKey( settings.raiseShortcutKey() );

    connect( ui->sce, SIGNAL(editTextChanged(QString)), this, SLOT(onSettingsChanged()));
#endif

    ui->cache->hide();

    ui->ssl->setVisible( QSslSocket::supportsSsl() );
    ui->ssl->setChecked( settings.value( "enableSsl", false ).toBool() );
    connect( ui->ssl, SIGNAL(clicked()), SLOT(onSettingsChanged()));
    connect( ui->proxySettings, SIGNAL(changed()), SLOT(onSettingsChanged()));
}

void
AdvancedSettingsWidget::saveSettings()
{
    qDebug() << "has unsaved changes?" << hasUnsavedChanges();

    if ( hasUnsavedChanges() )
    {
        AudioscrobblerSettings settings;
        settings.setRaiseShortcutKey( ui->sce->key() );
        settings.setRaiseShortcutModifiers( ui->sce->modifiers() );
        settings.setRaiseShortcutDescription( ui->sce->textValue() );

        aApp->setRaiseHotKey( ui->sce->modifiers(), ui->sce->key() );

        ui->proxySettings->save();

        settings.setValue( "enableSsl", ui->ssl->isChecked() );

        lastfm::ws::setScheme( ui->ssl->isChecked() ? lastfm::ws::Https : lastfm::ws::Http );
    }
}
