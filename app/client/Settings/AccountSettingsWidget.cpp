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

#include "ui_AccountSettingsWidget.h"
#include "AccountSettingsWidget.h"

#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/UnicornApplication.h"
#include "lib/unicorn/UnicornSession.h"
#include "lib/unicorn/UnicornSettings.h"

#include <lastfm/User.h>

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

using lastfm::User;


AccountSettingsWidget::AccountSettingsWidget( QWidget* parent )
    : SettingsWidget( parent ),
      ui( new Ui::AccountSettingsWidget )
{
    ui->setupUi( this );
    
    connect( ui->users, SIGNAL(userChanged() ), SLOT(onSettingsChanged() ) );
    connect( ui->users, SIGNAL(rosterUpdated() ), qApp, SIGNAL(rosterUpdated() ) );
}

void
AccountSettingsWidget::saveSettings()
{
    qDebug() << "has unsaved changes?" << hasUnsavedChanges();

    if ( hasUnsavedChanges() )
    {
        UserRadioButton* urb = qobject_cast<UserRadioButton*>( ui->users->checkedButton() );

        if ( urb && urb->user() != User().name() )
        {
            unicorn::Settings s;
            s.setValue( "Username", urb->user() );

            unicorn::UserSettings us( urb->user() );
            QString sessionKey = us.sessionKey();
            qobject_cast<unicorn::Application *>( qApp )->changeSession( urb->user(), sessionKey, true );
        }

        onSettingsSaved();
    }
}
