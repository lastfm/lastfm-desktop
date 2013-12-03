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

#include "TourLocationPage.h"
#include <QTimer>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QBoxLayout>

#include "FirstRunWizard.h"
#include "../Application.h"
#include "../Widgets/PointyArrow.h"

TourLocationPage::TourLocationPage()
    :m_flash( true )
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 20 );

    layout->addWidget( ui.image = new QLabel( this ), 0, Qt::AlignTop | Qt::AlignHCenter );
#ifdef Q_OS_MAC
    ui.image->setObjectName( "imagemac" );
#else
    ui.image->setObjectName( "imagewin" );
#endif
    layout->addWidget( ui.description = new QLabel( tr( "<p>The red arrow on your screen points to the location of the Last.fm Desktop App in your system tray.</p>"
                                                        "<p>Click the icon to quickly access radio play controls, share and tag track, edit your preferences and visit your Last.fm profile.</p>" ), this ),
                         0,
                         Qt::AlignTop);
    ui.description->setObjectName( "description" );
    ui.description->setWordWrap( true );
}


TourLocationPage::~TourLocationPage()
{
    if ( m_flashTimer )
        m_flashTimer->stop();

    if ( !m_normalIcon.isNull() )
        aApp->tray()->setIcon( m_normalIcon );

    delete m_arrow;
}


void
TourLocationPage::initializePage()
{
    delete m_arrow;
    m_arrow = new PointyArrow;
    delete m_flashTimer;
    m_flashTimer = new QTimer(this);
    m_flashTimer->setInterval( 300 );
    connect( m_flashTimer, SIGNAL(timeout()), SLOT(flashSysTray()));

#ifdef Q_OS_MAC
    setTitle( tr( "The Last.fm Desktop App in your menu bar" ) );
    ui.image->setPixmap( QPixmap( ":/graphic_location_MAC.png" ) );
#else
    setTitle( tr( "The Last.fm Desktop App in your system tray" ) );
    ui.image->setPixmap( QPixmap( ":/graphic_location_WIN.png" ) );
#endif

    QSystemTrayIcon* tray = aApp->tray();
    m_arrow->pointAt( QPoint( tray->geometry().left() + (tray->geometry().width() / 2.0f ), tray->geometry().top() + (tray->geometry().height() / 2.0f ) ));
    m_flashTimer->start();
    m_normalIcon = tray->icon();
    m_transparentIcon = QPixmap( ":22x22_transparent.png" ).scaled( m_normalIcon.availableSizes().first());
    m_flash = false;

    wizard()->setButton( FirstRunWizard::NextButton, tr( "Continue" ) );

    if ( wizard()->canGoBack() )
        wizard()->setButton( FirstRunWizard::BackButton, tr( "<< Back" ) );
}

void
TourLocationPage::cleanupPage()
{
    delete m_arrow;
    delete m_flashTimer;
    aApp->tray()->setIcon( m_normalIcon );
}

void
TourLocationPage::flashSysTray()
{
    QSystemTrayIcon* tray = aApp->tray();

    if( m_flash )
        tray->setIcon( m_transparentIcon );
    else
        tray->setIcon( m_normalIcon );

    m_flash = !m_flash;
}
