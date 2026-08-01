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
#include "LoginContinueDialog.h"

#include <QtGui>
#include <QtWidgets>
#include <QDialogButtonBox>

LoginContinueDialog::LoginContinueDialog( QWidget* parent )
    : QDialog( parent ), m_subscriber( true )
{
    setWindowModality( Qt::ApplicationModal );

    QVBoxLayout* layout = new QVBoxLayout( this );

    layout->addWidget( ui.title = new QLabel( tr("Are we done?") ) );
    ui.title->setObjectName( "title" );

    layout->addWidget( ui.description = new QLabel( tr("Click OK once you have approved this app.") ) );
    ui.title->setObjectName( "description" );
    ui.title->setWordWrap( true );

    layout->addWidget( ui.buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok ) );

    connect( ui.buttonBox, SIGNAL(accepted()), SLOT(accept()) );
    connect( ui.buttonBox, SIGNAL(rejected()), SLOT(reject()) );
}

