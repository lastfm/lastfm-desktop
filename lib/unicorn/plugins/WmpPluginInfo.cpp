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

#include <QStringList>

#include "WmpPluginInfo.h"

unicorn::WmpPluginInfo::WmpPluginInfo( QObject* parent )
    :IPluginInfo( parent )
{}

unicorn::Version
unicorn::WmpPluginInfo::version() const
{
    return Version( 2, 1, 0, 8 );
}

QString
unicorn::WmpPluginInfo::name() const
{
    return "Windows Media Player";
}

QString
unicorn::WmpPluginInfo::displayName() const
{
    return QString( "MPlayer2" );
}

QString
unicorn::WmpPluginInfo::processName() const
{
    return QString( "wmplayer.exe" );
}

QString
unicorn::WmpPluginInfo::id() const
{
    return "wmp";
}

bool
unicorn::WmpPluginInfo::isAppInstalled() const
{
    return QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\MediaPlayer\\Preferences", QSettings::NativeFormat).contains("FirstRun");
}

unicorn::IPluginInfo::BootstrapType
unicorn::WmpPluginInfo::bootstrapType() const
{
    return PluginBootstrap;
}

QString
unicorn::WmpPluginInfo::pluginInstaller() const
{
    return "WmpPluginSetup_2.1.0.8.exe";
}
