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

#include <QtGui>
#include <QLabel>

#include "lib/unicorn/dialogs/ScrobbleConfirmationDialog.h"
#include "lib/unicorn/widgets/Label.h"
#include "lib/unicorn/DesktopServices.h"

#include "MessageBar.h"

MessageBar::MessageBar( QWidget* parent )
    :QFrame( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    layout->addWidget( ui.icon = new QLabel() );
    ui.icon->setObjectName( "icon" );

    layout->addWidget( ui.message = new QLabel() );
    ui.message->setObjectName( "message" );

    layout->addStretch();

    layout->addWidget( ui.close = new QPushButton() );
    ui.close->setObjectName( "close" );

    connect( ui.message, SIGNAL(linkActivated(QString)), SLOT(onLinkActivated(QString)));
    connect( ui.close, SIGNAL(clicked()), SLOT(onCloseClicked()));

    connect( qApp, SIGNAL(showMessage(QString,QString)), SLOT(show(QString,QString)));
    connect( qApp, SIGNAL(error(QString)), SLOT(show(QString)));

    hide();
}

void
MessageBar::addTracks( const QList<lastfm::Track>& tracks )
{
    m_tracks << tracks;
}

const QList<lastfm::Track>&
MessageBar::tracks() const
{
    return m_tracks;
}

void
MessageBar::show( const QString& message, const QString& id, int timeout )
{
    if ( !m_timeout )
    {
        m_timeout = new QTimer( this );
        connect( m_timeout, SIGNAL(timeout()), SLOT(onCloseClicked()) );
    }

    if ( timeout != -1 )
        m_timeout->start( timeout * 1000 );
    else
        m_timeout->stop();


    setObjectName( id );
    ui.message->setText( unicorn::Label::boldLinkStyle( message, Qt::black ) );

    style()->polish( this );
    style()->polish( ui.icon );

    QWidget::show();
}

void
MessageBar::onCloseClicked()
{
    m_tracks.clear();
    hide();
}

void
MessageBar::onLinkActivated( const QString& link )
{
    if ( link == "tracks" )
    {
        // always sort the tracks before displaying them
        qSort ( m_tracks.begin(), m_tracks.end() );

        // Show a dialog with the tracks
        ScrobbleConfirmationDialog confirmDialog( m_tracks );
        confirmDialog.setReadOnly();
        confirmDialog.exec();
    }

    else if ( link == "plugin" )
    {
#ifdef Q_OS_MAC
        if ( !m_installer )
            m_installer = new unicorn::ITunesPluginInstaller( this );

        m_installer->install();
#elif defined( Q_OS_WIN )
        if ( !m_itw )
            m_itw = new unicorn::ITunesPluginInfo( this );

        m_itw->setVerbose( true );
        m_itw->doInstall();
#endif
    }

    else
    {
        // this should be a url so open it in the browser
        unicorn::DesktopServices::openUrl( link );
    }
}

