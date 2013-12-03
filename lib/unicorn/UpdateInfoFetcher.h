/*
   Copyright 2009-2012 Last.fm Ltd. 
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

#ifndef UPDATE_INFO_FETCHER_H_
#define UPDATE_INFO_FETCHER_H_
#include <QObject>
#include "lib/DllExportMacro.h"
#include <QUrl>
#include <QDir>

namespace lastfm{ class XmlQuery; }

class UNICORN_DLLEXPORT Plugin
{
public:
    enum BootstrapType{ NoBootstrap, ClientBootstrap, PluginBootstrap };

    Plugin(): m_valid( false ){}
    Plugin( const lastfm::XmlQuery& );

    QString toString() const { 
         return QString( "%1 (%2 - %3)\n\tinstallDir: %4\n\targs:%5\n\tregDisplayName: %6" )
                    .arg( m_name )
                    .arg( m_minVersion )
                    .arg( m_maxVersion )
                    .arg( m_installDir.path() )
                    .arg( m_args )
                    .arg( m_regDisplayName );
    }
    
    QString name() const { return m_name; }
    QString regDisplayName() const { return m_regDisplayName; }

    bool isValid() const { return m_valid; }
    bool isInstalled() const;
    bool isPluginInstalled() const;
    bool canBootstrap() const;

private:
    QString m_name;
    QString m_id;
    QUrl m_url;
    QDir m_installDir;
    QString m_args;
    QString m_minVersion;
    QString m_maxVersion;
    QString m_regDisplayName;

    BootstrapType m_bootstrapType;

    bool m_valid;
};


class UNICORN_DLLEXPORT UpdateInfoFetcher : public QObject
{
Q_OBJECT
public:
    UpdateInfoFetcher( class QNetworkReply* reply, QObject* parent = 0 );
    static QNetworkReply* fetchInfo();
    const QList<Plugin>& plugins() const{ return m_plugins; }

private:
    QList<Plugin> m_plugins;
};

#endif //UPDATE_INFO_FETCHER_H_
