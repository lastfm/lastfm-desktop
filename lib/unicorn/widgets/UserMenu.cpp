/*
   Copyright 2010-2012 Last.fm Ltd.
      - Primarily authored by Jono Cole and Michael Coffey

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

#include <QDebug>
#include <QTimer>
#include <QDesktopServices>

#include <lastfm/User.h>
#include <lastfm/UrlBuilder.h>

#include "lib/unicorn/UnicornApplication.h"
#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/DesktopServices.h"

#include "UserMenu.h"

UserMenu::UserMenu( QWidget* p )
    :QMenu( p )
{
    connect( qApp, SIGNAL(rosterUpdated()), SLOT(refresh()));
    connect( qApp, SIGNAL(sessionChanged(unicorn::Session) ), SLOT( onSessionChanged( unicorn::Session) ) );

    refresh();
}

void
UserMenu::onSessionChanged( const unicorn::Session& session )
{
    foreach( QAction* a, actions() )
        if( a->text() == session.user().name() )
            a->setChecked( true );

    // show the subscribe link if they don't have radio and could get it by subscribing
    m_subscribe->setVisible( !session.youRadio() && session.subscriberRadio() );
}

void
UserMenu::onTriggered( QAction* a )
{
    unicorn::UserSettings us( a->text() );

    QString username = a->text();
    QString sessionKey = us.sessionKey();
    QMetaObject::invokeMethod( qApp, "changeSession",
                                     Q_ARG( const QString, username ),
                                     Q_ARG( const QString, sessionKey ) );

    //Refresh the user list to be certain that
    //the correct current user is checked.
    //(ie. the user change could be cancelled after confirmation.)
    refresh();

}

void
UserMenu::manageUsers()
{
    //This is required so that the menu popup call can be completed
    //in the event loop before the manage users dialog is displayed.
    //(The manageUsers method may emit a signal which causes the menu
    // to be destroyed)
    QTimer::singleShot( 0, qApp, SLOT(manageUsers()) );
}

void
UserMenu::subscribe()
{
    unicorn::DesktopServices::openUrl( lastfm::UrlBuilder( "subscribe" ).url() );
}

void
UserMenu::refresh()
{
    clear();

    m_subscribe = addAction( tr( "Subscribe" ), this, SLOT(subscribe()));

    addSeparator();

    QActionGroup* ag = new QActionGroup( this );

    foreach( User u, unicorn::Settings().userRoster() )
    {
        QAction* a = ag->addAction( new QAction( u.name(), this ));
        addAction( a );
        if( u == User()) a->setChecked( true );
        a->setCheckable( true );
    }

    ag->setExclusive( true );
    connect( ag, SIGNAL(triggered(QAction*)), SLOT( onTriggered(QAction*)));

    onSessionChanged( qobject_cast<unicorn::Application*>(qApp)->currentSession() );
}

