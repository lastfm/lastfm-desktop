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

#include "UpdateInfoFetcher.h"
#include <lastfm/ws.h>
#include <lastfm/XmlQuery.h>
#include <QDebug>
#include <QSettings>
#include <QString>

#ifdef WIN32
    static const char *PLATFORM = "win";
#elif defined Q_WS_X11
    static const char *PLATFORM = "linux";
#elif defined Q_WS_MAC
    static const char *PLATFORM = "mac";
#else
    static const char *PLATFORM = "unknown";
#endif

Plugin::Plugin( const XmlQuery& query )
       :m_bootstrapType( NoBootstrap ),
        m_valid( true )

{
    m_name = query.attribute( "name" );
    m_id = query.attribute( "id" );
    m_url = QUrl( query["Url"].text() );
    m_installDir = QDir( query[ "InstallDir" ].text() );
    m_args = query["Args"].text();
    m_minVersion = query["MinVersion"].text();
    m_maxVersion = query["MaxVersion"].text();
    m_regDisplayName = query["RegDisplayName"].text();

    QString bs = query["Bootstrap"].text();
    if( bs.compare( "Client", Qt::CaseInsensitive ) == 0) {
        m_bootstrapType = ClientBootstrap;
    } else if( bs.compare( "Plugin", Qt::CaseInsensitive ) == 0) {
        m_bootstrapType = PluginBootstrap;
    }
}


bool 
Plugin::isInstalled() const
{
#ifdef Q_OS_WIN
    QSettings s("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\", QSettings::NativeFormat );
    foreach( QString group, s.childGroups()) {
        s.beginGroup( group );
            QString name = s.value( "DisplayName" ).toString();
            if( name.contains( m_regDisplayName ) ||
                group.contains( m_regDisplayName )) {
                    return true;
            }
        s.endGroup();
    }
    return false;
#elif defined Q_OS_MAC
    return true;
#endif
}


bool 
Plugin::isPluginInstalled() const
{
#ifdef Q_OS_WIN
    QSettings s( "HKEY_LOCAL_MACHINE\\SOFTWARE\\Last.fm\\Client\\Plugins", QSettings::NativeFormat );
    return s.childGroups().contains( m_id );
#elif defined Q_OS_MAC
    return true;
#endif
}


bool
Plugin::canBootstrap() const
{
    return m_bootstrapType != NoBootstrap;
}


UpdateInfoFetcher::UpdateInfoFetcher( QNetworkReply* reply, QObject* parent )
                  :QObject( parent )
{
    XmlQuery xq;
    xq.parse( reply );

    QList<XmlQuery> plugins = xq.children( "Plugin" );
    foreach( const XmlQuery& plugin, plugins ) {
        m_plugins << Plugin( plugin );
    }
}


QNetworkReply* 
UpdateInfoFetcher::fetchInfo() //static
{
    QString url = QString( "http://%1/ass/upgrade.xml.php?platform=%2&lang=en" ).arg( lastfm::ws::host(), PLATFORM );
    QNetworkRequest req( url );
    return lastfm::nam()->get( req );
}
