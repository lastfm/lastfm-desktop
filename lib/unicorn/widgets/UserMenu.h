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

#ifndef USER_MENU_H_
#define USER_MENU_H_

#include <QMenu>
#include <QPointer>

#include "lib/DllExportMacro.h"
#include "lib/unicorn/UnicornSession.h"

class UNICORN_DLLEXPORT UserMenu : public QMenu
{
    Q_OBJECT
public:
    UserMenu( QWidget* p = 0 );

protected slots:
    void onSessionChanged( const unicorn::Session& session );

    void onTriggered( QAction* a );
    void manageUsers();
    void refresh();
    void subscribe();

private:
    QPointer<QAction> m_subscribe;
};

#endif
