/*
   Copyright 2010-2012 Last.fm Ltd.
      - Primarily authored by Jono Cole, William Viana Soares and Michael Coffey

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

#ifndef USER_COMBO_SELECTOR_H_
#define USER_COMBO_SELECTOR_H_

#include <QComboBox>
#include <QStandardItemModel>
#include <lastfm/User.h>
#include "lib/DllExportMacro.h"
#include "lib/unicorn/UnicornApplication.h"
#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/UnicornSession.h"

class UNICORN_DLLEXPORT UserComboSelector : public QComboBox
{
    Q_OBJECT
public:
    UserComboSelector( QWidget* p = 0 )
    :QComboBox( p )
    {
        setStyle( qApp->style());
        refresh();


        connect( this, SIGNAL( activated( int)), SLOT( onActivated( int )));
        connect( qApp, SIGNAL( rosterUpdated()), SLOT( refresh()));
        connect( qApp, SIGNAL( sessionChanged( unicorn::Session, unicorn::Session ))
                     , SLOT( onSessionChanged( unicorn::Session )));

    }

protected:
    void changeUser( const QString& username )
    {
        unicorn::UserSettings us( username );

        QString sessionKey = us.value( "sessionKey", "" ).toString();
        QMetaObject::invokeMethod( qApp, "changeSession",
                                         Q_ARG( const QString, username ),
                                         Q_ARG( const QString, sessionKey ) );

    }

protected slots:
    void onSessionChanged( unicorn::Session& s )
    {
        int index = findText( s.user().name() );
        setCurrentIndex( index );
    }

    void onActivated( int index )
    {
        if( itemData( index ).toBool())
            return changeUser( itemText( index ));
        
        //Reset current user as selected item
        refresh();

        //show manage users dialog
        QMetaObject::invokeMethod( qApp, "manageUsers", Qt::DirectConnection );

        //Reset user list / selected user
        refresh();
    }


    void refresh()
    {
        clear();
        foreach( User u, unicorn::Settings().userRoster() ) {
            addItem( u.name(), true );
        }

        insertSeparator( count());  //why is there no addSeparator?!
        addItem( "Manage Users", false );

        onSessionChanged( qobject_cast<unicorn::Application*>(qApp)->currentSession() );
    }
};

#endif
