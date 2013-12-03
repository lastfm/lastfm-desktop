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

#ifndef PLUGIN_LIST_H_
#define PLUGIN_LIST_H_

#include <QObject>

#include "WinampPluginInfo.h"
#include "WmpPluginInfo.h"
#include "ITunesPluginInfo.h"
#include "Foobar09PluginInfo.h"

#include <lib/DllExportMacro.h>

namespace unicorn
{

class UNICORN_DLLEXPORT PluginList : public QObject
{
    Q_OBJECT
public:
    PluginList( QObject* parent = 0 ) : QObject( parent )
    {
        m_plugins << (new ITunesPluginInfo( this ));
        m_plugins << (new WmpPluginInfo( this ));
        m_plugins << (new WinampPluginInfo( this ));
        m_plugins << (new FooBar09PluginInfo( this ));
    }

    QList<IPluginInfo*> availablePlugins() const;
    QList<IPluginInfo*> installedPlugins() const;
    QList<IPluginInfo*> bootstrappablePlugins() const;
    QList<IPluginInfo*> installList() const;

    QList<IPluginInfo*> updatedList() const;
    QList<IPluginInfo*> supportedList() const;
    QString availableDescription() const;

    IPluginInfo* pluginById( const QString& id ) const;

private:
    QList<IPluginInfo*> m_plugins;
};

}

#endif //PLUGIN_LIST_H_
