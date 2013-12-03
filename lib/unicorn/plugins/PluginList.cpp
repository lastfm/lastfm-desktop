/*
   Copyright 2010-2013 Last.fm Ltd.
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

#include "PluginList.h"
#include <QStringList>

QList<unicorn::IPluginInfo*>
unicorn::PluginList::availablePlugins() const
{
    QList<IPluginInfo*> ret;
    foreach( IPluginInfo* plugin, m_plugins )
    {
        if( plugin->isInstalled() )
            continue;
        if( plugin->isAppInstalled() )
            ret << plugin;
    }
    return ret;
}

QList<unicorn::IPluginInfo*>
unicorn::PluginList::installedPlugins() const
{
    QList<IPluginInfo*> ret;
    foreach( IPluginInfo* plugin, m_plugins )
    {
        if( plugin->isInstalled())
            ret << plugin;
    }
    return ret;
}

QList<unicorn::IPluginInfo*>
unicorn::PluginList::bootstrappablePlugins() const
{
    QList<IPluginInfo*> ret;
    foreach( IPluginInfo* plugin, installedPlugins() )
    {
        if( plugin->isInstalled() && plugin->canBootstrap() )
            ret << plugin;
    }
    return ret;
}

QList<unicorn::IPluginInfo*>
unicorn::PluginList::supportedList() const
{
    QList<IPluginInfo*> ret;
    foreach( IPluginInfo* i, m_plugins )
        ret << i;

    return ret;
}

QList<unicorn::IPluginInfo*>
unicorn::PluginList::installList() const
{
    QList<IPluginInfo*> ret;
    foreach( IPluginInfo* i, m_plugins )
    {
        if ( i->install() )
            ret << i;
    }

    return ret;
}

QList<unicorn::IPluginInfo*>
unicorn::PluginList::updatedList() const
{
    QList<IPluginInfo*> ret;
    foreach( IPluginInfo* i, m_plugins )
    {
        if ( i->isInstalled() && i->version() > i->installedVersion() )
            ret << i;
    }

    return ret;
}

QString
unicorn::PluginList::availableDescription() const
{
    QStringList mediaPlayers;
    
    foreach( IPluginInfo* i, supportedList() )
        mediaPlayers << i->name();

    QString ret = mediaPlayers.takeLast();
    
    if( mediaPlayers.count() > 0 )
        ret = mediaPlayers.join( ", " ) + " or " + ret;

    return ret;
}

unicorn::IPluginInfo*
unicorn::PluginList::pluginById( const QString& id ) const
{
    foreach( IPluginInfo* plugin, m_plugins )
    {
        if( !plugin->id().compare( id ) )
            return plugin;
    }

    return NULL;
}
