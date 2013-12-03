/*
   Copyright 2009 Last.fm Ltd.
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

#ifndef PLAYDAR_API_H
#define PLAYDAR_API_H

#include "TPlaydarApi.hpp"
#include <QUrl>
#include <QString>

typedef QList< QPair<QString,QString> > ParamList;

class PlaydarApiQtPolicy
{
public:
    static void paramsAdd(ParamList& params, const QString& name, const QString& value)
    {
        params.append(QPair< QString, QString>(name, value));
    }

    static QUrl createUrl(const QString& base, const QString& path, const ParamList& params)
    {
        QUrl url(base + path);
        url.setQueryItems(params);
        return url;
    }
};


typedef TPlaydarApi<QString, QUrl, ParamList, PlaydarApiQtPolicy> PlaydarApi;

#endif

