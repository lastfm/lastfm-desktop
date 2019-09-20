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
#include "AboutDialog.h"
#include <QApplication>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

#include "ui_AboutDialog.h"


AboutDialog::AboutDialog( QWidget* parent )
           : QDialog( parent ),
             ui( new Ui::AboutDialog )
{
    Q_ASSERT( qApp->applicationVersion().size() );

    ui->setupUi( this );

    ui->appName->setText( "<b>" + qApp->applicationName() );
    ui->qtVersion->setText(  tr( "%1 (built on Qt %2)"  ).arg( qApp->applicationVersion(), qVersion() ) );

    ui->lastfmLink->setText(  "<a href='http://www.last.fm'>www.last.fm</a>" );
    ui->ircLink->setText(  "<a href='https://getsatisfaction.com/lastfm/categories/lastfm_apps'>Last.fm Client Support</a>" );

    ui->copyright->setText( QString::fromUtf8("™ & © 2005 - 2019 Last.fm Limited") );
}
