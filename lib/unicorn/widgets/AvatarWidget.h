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

#ifndef AVATARWIDGET_H
#define AVATARWIDGET_H

#include <lastfm/User.h>

#include "lib/unicorn/widgets/HttpImageWidget.h"

#include "lib/DllExportMacro.h"


class UNICORN_DLLEXPORT AvatarWidget : public HttpImageWidget
{
    Q_OBJECT
public:
    explicit AvatarWidget( QWidget* parent = 0 );

    void setUser( const lastfm::User& user );

private:
    void paintEvent( QPaintEvent *paintEvent );

private:
    lastfm::User m_user;
};

#endif // AVATARWIDGET_H
