/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Jono Cole and Micahel Coffey

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

#include <QAction>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSizeGrip>

#include "lib/unicorn/widgets/Label.h"

#include "StatusBar.h"

#include "../Services/ScrobbleService.h"
#include "../MediaDevices/DeviceScrobbler.h"
#include "../Application.h"

StatusBar::StatusBar( QWidget* parent )
    :QStatusBar( parent ),
      m_online( false )
{
    addWidget( ui.widget = new QFrame( this) );
    QHBoxLayout* widgetLayout = new QHBoxLayout( ui.widget );
    widgetLayout->setContentsMargins( 0, 0, 0, 0 );
    widgetLayout->setSpacing( 0 );

    widgetLayout->addWidget( ui.cog = new QPushButton(this), 0, Qt::AlignVCenter);
    ui.cog->setObjectName( "cog" );
    ui.cog->setAttribute( Qt::WA_LayoutUsesWidgetRect );

    widgetLayout->addWidget( ui.message = new unicorn::Label(this), 1, Qt::AlignVCenter);
    ui.message->setObjectName( "message" );
    ui.message->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui.message->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    setStatus();

    addPermanentWidget( ui.scrobbleWidget = new QFrame( this ));
    ui.scrobbleWidget->setObjectName("scrobbleWidget");
    QHBoxLayout* scrobbleWidgetLayout = new QHBoxLayout( ui.scrobbleWidget );
    scrobbleWidgetLayout->setContentsMargins( 50, 0, 0, 0 );
    scrobbleWidgetLayout->setSpacing( 0 );

    bool isScrobblingOff = unicorn::UserSettings().value("scrobblingOn", true).toBool();

    scrobbleWidgetLayout->addWidget( ui.scrobbleIcon = new QLabel(this) );
    ui.scrobbleIcon->setObjectName("scrobbleIcon");
    ui.scrobbleIcon->setHidden(isScrobblingOff);

    scrobbleWidgetLayout->addWidget( ui.scrobbleMessage = new unicorn::Label(this), 1, Qt::AlignVCenter);
    ui.scrobbleMessage->setObjectName("scrobbleMessage");
    ui.scrobbleMessage->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui.scrobbleMessage->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
    ui.scrobbleMessage->setStyleSheet("QLabel { color : red; }");
    ui.scrobbleMessage->setText( tr("Scrobbling is off") );
    ui.scrobbleMessage->setHidden(isScrobblingOff);

    aApp->isInternetConnectionUp() ? onConnectionUp() : onConnectionDown();

    connect( aApp, SIGNAL( internetConnectionDown() ), SLOT( onConnectionDown() ) );
    connect( aApp, SIGNAL( internetConnectionUp() ), SLOT( onConnectionUp() ) );

    connect( this, SIGNAL(messageChanged(QString)), SLOT(onMessagedChanged(QString)));
    connect( aApp, SIGNAL(scrobbleToggled(bool)), ui.scrobbleMessage, SLOT(setHidden(bool)));
    connect( aApp, SIGNAL(scrobbleToggled(bool)), ui.scrobbleIcon, SLOT(setHidden(bool)));

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)));

    connect( ui.cog, SIGNAL(clicked()), aApp, SLOT(onPrefsTriggered()));
}


void
StatusBar::onSessionChanged( const unicorn::Session& /*session*/ )
{
    setStatus();
}

void
StatusBar::onMessagedChanged( const QString& message )
{
    if ( message.isEmpty() )
        setStatus();
}

void
StatusBar::setStatus()
{
    ui.message->setText( tr("%1 (%2)").arg( lastfm::ws::Username, m_online ? tr( "Online" ) : tr( "Offline" ) ));
}

void
StatusBar::onConnectionUp()
{
    m_online = true;
    setStatus();
}

void
StatusBar::onConnectionDown()
{
    m_online = false;
    setStatus();
}

