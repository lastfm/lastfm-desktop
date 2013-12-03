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

#include "UserManagerDialog.h"

#include <lastfm/User.h>

#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/widgets/UserManagerWidget.h"

#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>


UserManagerDialog::UserManagerDialog( QWidget* parent )
            :QDialog( parent )
{
    m_users = new UserManagerWidget( this );
    QVBoxLayout* layout = new QVBoxLayout();
    QHBoxLayout* actionButtons = new QHBoxLayout();
    QDialogButtonBox* bb;
    actionButtons->addWidget( bb = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ));

    layout->addWidget( m_users );
    layout->addLayout( actionButtons );
    setLayout( layout );

    connect( m_users, SIGNAL( rosterUpdated() ), this, SIGNAL( rosterUpdated() ) );
    connect( bb, SIGNAL( accepted()), SLOT( onAccept()));
    connect( bb, SIGNAL( rejected()), SLOT( reject()));
}


UserManagerDialog::~UserManagerDialog()
{
}

void 
UserManagerDialog::onAccept()
{
    unicorn::Settings s;
    UserRadioButton* urb = m_users->checkedButton();

    if( !urb || urb->user() == User().name())
        return QDialog::reject();

    s.setValue( "Username", urb->user());
    QDialog::accept();
}
