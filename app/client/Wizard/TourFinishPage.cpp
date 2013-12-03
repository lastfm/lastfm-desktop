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

#include <QHBoxLayout>
#include <QLabel>

#include "TourFinishPage.h"

#include "../Application.h"
#include "../Bootstrapper/AbstractBootstrapper.h"

TourFinishPage::TourFinishPage()
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 20 );

    layout->addWidget( ui.image = new QLabel( this ), 0, Qt::AlignTop | Qt::AlignHCenter );
    ui.image->setObjectName( "image" );
    layout->addWidget( ui.description = new QLabel( this ), 0, Qt::AlignTop );
    ui.description->setObjectName( "description" );
    ui.description->setWordWrap( true );

    connect( aApp, SIGNAL(bootstrapDone(AbstractBootstrapper::BootstrapStatus)), SLOT(onBootstrapDone(AbstractBootstrapper::BootstrapStatus)));
}

void
TourFinishPage::setDescription( FirstRunWizard::BootstrapState state, AbstractBootstrapper::BootstrapStatus status )
{
    if ( state == FirstRunWizard::BootstrapFinished )
    {
        if ( status == AbstractBootstrapper::Bootstrap_Ok )
        {
            ui.description->setText( tr( "<p>Now you're ready to get started! Just click <strong>Finish</strong> and start exploring.</p>"
                                         "<p>We've also finished importing your listening history and have added it to your Last.fm profile.</p>"
                                         "<p>Thanks for installing the Last.fm Desktop App, we hope you enjoy using it!</p>") );
        }
        else
        {
            QMap<AbstractBootstrapper::BootstrapStatus, QString> reasons;
            reasons[AbstractBootstrapper::Bootstrap_UploadError] = tr( "there was an upload error" );
            reasons[AbstractBootstrapper::Bootstrap_Denied] = tr( "the submission was denied by Last.fm" );
            reasons[AbstractBootstrapper::Bootstrap_Spam] = tr( "it was detected as spam (too high playcounts?)" );
            reasons[AbstractBootstrapper::Bootstrap_Cancelled] = tr( "the submission was cancelled" );

            ui.description->setText( tr( "<p>Now you're ready to get started! Just click <strong>Finish</strong> and start exploring.</p>"
                                         "<p>Importing your listening history to Last.fm failed because %1. Sorry about that!</p>"
                                         "<p>Thanks for installing the Last.fm Desktop App, we hope you enjoy using it!</p>").arg( reasons[status] ) );
        }
    }
    else if ( state == FirstRunWizard::BootstrapStarted )
    {
        ui.description->setText( tr( "<p>Now you're ready to get started! Just click <strong>Finish</strong> and start exploring.</p>"
                                     "<p>We're still importing your listening history and it will be added to your Last.fm profile soon.</p>"
                                     "<p>Thanks for installing the Last.fm Desktop App, we hope you enjoy using it!</p>") );
    }
    else
    {
        ui.description->setText( tr( "<p>Now you're ready to get started! Just click <strong>Finish</strong> and start exploring.</p>"
                                     "<p>Thanks for installing the Last.fm Desktop App, we hope you enjoy using it!</p>") );
    }
}

void
TourFinishPage::initializePage()
{
    setTitle( tr( "That's it, you're good to go!" ) );

    setDescription( wizard()->bootstrapState(), wizard()->bootstrapStatus() );

    wizard()->setButton( FirstRunWizard::FinishButton, tr( "Finish" ) );

    if ( wizard()->canGoBack() )
        wizard()->setButton( FirstRunWizard::BackButton, tr( "<< Back" ) );
}

void
TourFinishPage::cleanupPage()
{
}


void
TourFinishPage::onBootstrapDone( AbstractBootstrapper::BootstrapStatus status )
{
    setDescription( FirstRunWizard::BootstrapFinished, status);
}


