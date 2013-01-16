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
#include "FriendsPicker.h"
#include <QLineEdit>

#include <lastfm/User.h>
#include <QDebug>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QVBoxLayout>


FriendsPicker::FriendsPicker( const User& user )
{    
    qDebug() << user;
    
    QVBoxLayout* v = new QVBoxLayout( this );
    QLineEdit* lineEdit = new QLineEdit;
#if QT_VERSION >= 0x040700
    // The placeholder property was introduced in Qt 4.7
    lineEdit->setPlaceholderText( tr("Search your friends") );
#endif
    v->addWidget( lineEdit );
    v->addWidget( ui.list = new QListWidget );
    v->addWidget( ui.buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) );
     
    setWindowTitle( tr("Browse Friends") );
    
    connect( user.getFriends(), SIGNAL(finished()), SLOT(onGetFriendsReturn()) );
    
    connect( ui.buttons, SIGNAL(accepted()), SLOT(accept()) );
    connect( ui.buttons, SIGNAL(rejected()), SLOT(reject()) );
}


void
FriendsPicker::onGetFriendsReturn()
{
    foreach (User u, User::list( (QNetworkReply*)sender() ).users())
        ui.list->addItem( u );
}


QList<User>
FriendsPicker::selection() const
{
    return QList<User>();
}
