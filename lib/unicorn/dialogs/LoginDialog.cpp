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
#include "LoginDialog.h"
#include "lib/unicorn/QMessageBoxBuilder.h"
#include <QtGui>
#include <QtWidgets>

LoginDialog::LoginDialog( QWidget* parent )
            :QDialog( parent )
{
    setWindowModality( Qt::ApplicationModal );

    QVBoxLayout* layout = new QVBoxLayout( this );

    layout->addWidget( ui.title = new QLabel( tr("Last.fm needs your permission first!") ) );
    ui.title->setObjectName( "title" );

    layout->addWidget( ui.description = new QLabel( tr("This application needs your permission to connect to your Last.fm profile.  Click OK to go to the Last.fm website and do this.") ) );
    ui.description->setWordWrap( true );
    ui.title->setObjectName( "description" );
    ui.title->setWordWrap( true );

    layout->addWidget( ui.buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) );

    connect( ui.buttonBox, SIGNAL(accepted()), SLOT(accept()) );
    connect( ui.buttonBox, SIGNAL(rejected()), SLOT(reject()) );
}



