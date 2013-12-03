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
#include <QDesktopServices>
#include <QAbstractButton>

#include <lastfm/UrlBuilder.h>

#include "lib/unicorn/DesktopServices.h"

#include "../Application.h"

#include "FirstRunWizard.h"
#include "TourRadioPage.h"

TourRadioPage::TourRadioPage()
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 20 );

    layout->addWidget( ui.image = new QLabel( this ), 0, Qt::AlignTop | Qt::AlignHCenter );
    ui.image->setObjectName( "image" );
    layout->addWidget( ui.description = new QLabel( "", this ), 0, Qt::AlignTop);
    ui.description->setObjectName( "description" );
    ui.description->setWordWrap( true );
}


void
TourRadioPage::initializePage()
{
    // we should only get here, if the user is able to subscribe to listen to radio

    if ( aApp->currentSession().youRadio() )
    {
        setTitle( tr( "Listen to non-stop, personalised radio" ) );
        ui.description->setText( tr( "<p>Use the Last.fm Desktop App to listen to personalised radio based on the music you want to hear.</p>"
                                     "<p>Every play of every Last.fm station is totally different, from stations based on artists and tags to brand new recommendations tailored to your music taste.</p>" ) );
    }
    else
    {
        setTitle( tr( "Subscribe and listen to non-stop, personalised radio" ) );
        ui.description->setText( tr( "<p>Subscribe to Last.fm and use the Last.fm Desktop App to listen to personalised radio based on the music you want to hear.</p>"
                                     "<p>Every play of every Last.fm station is totally different, from stations based on artists and tags to brand new recommendations tailored to your music taste.</p>" ) );

        QAbstractButton* custom = wizard()->setButton( FirstRunWizard::CustomButton, tr( "Subscribe" ) );
        connect( custom, SIGNAL(clicked()), SLOT(subscribe()));
    }

    wizard()->setButton( FirstRunWizard::NextButton, tr( "Continue" ) );
    if ( wizard()->canGoBack() )
        wizard()->setButton( FirstRunWizard::BackButton, tr( "<< Back" ) );
    wizard()->setButton( FirstRunWizard::SkipButton, tr( "Skip Tour >>" ) );
}

void
TourRadioPage::cleanupPage()
{
}

void
TourRadioPage::subscribe()
{
    unicorn::DesktopServices::openUrl( lastfm::UrlBuilder( "subscribe" ).url() );
}
