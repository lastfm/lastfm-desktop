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

#include <lastfm/User.h>

#include "lib/unicorn/DesktopServices.h"

#include "../Application.h"

#include "BetaDialog.h"
#include "ui_BetaDialog.h"

BetaDialog::BetaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BetaDialog)
{
    ui->setupUi(this);

    setAttribute( Qt::WA_DeleteOnClose );

    ui->description->setText( "This is a beta version of the new Last.fm Desktop App. Please be gentle!" );

    if ( aApp->currentSession().user().type() == lastfm::User::TypeStaff )
    {
        ui->feedback->setText( "If you've' noticed a problem, create a Jira issue and we'll fix it immediately. Thanks!" );

    }
    else
    {
        ui->createIssue->setText( tr( "Visit Audioscrobbler Beta group" ) );
        ui->feedback->setText( "If you've noticed a problem, please visit our group and leave us some feedback. Thanks!" );
    }

    connect( ui->createIssue, SIGNAL(clicked()), SLOT(createIssue()) );
}

void
BetaDialog::createIssue()
{
    if ( aApp->currentSession().user().type() == lastfm::User::TypeStaff )
        unicorn::DesktopServices::openUrl( QUrl( "https://jira.last.fm/secure/CreateIssue.jspa?pid=10011&issuetype=1&Create=Create" ) );
    else
        unicorn::DesktopServices::openUrl( QUrl( "http://www.last.fm/group/Audioscrobbler+Beta" ) );
}

BetaDialog::~BetaDialog()
{
    delete ui;
}

