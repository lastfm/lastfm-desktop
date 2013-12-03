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

#include <QSettings>
#include <QStringList>

#include "FooBar09PluginInfo.h"

unicorn::FooBar09PluginInfo::FooBar09PluginInfo( QObject* parent )
    :IPluginInfo( parent )
{}

unicorn::Version
unicorn::FooBar09PluginInfo::version() const
{
    return Version( 2, 3, 1, 3 );
}

QString
unicorn::FooBar09PluginInfo::name() const
{
    return "foobar2000";
}

QString
unicorn::FooBar09PluginInfo::displayName() const
{
    return QString( "foobar2000" );
}

QString
unicorn::FooBar09PluginInfo::processName() const
{
    return QString( "foobar2000.exe" );
}

QString
unicorn::FooBar09PluginInfo::id() const
{
    return QString( "foo3" );
}

bool
unicorn::FooBar09PluginInfo::isAppInstalled() const
{
    return QSettings("HKEY_CURRENT_USER\\Software\\foobar2000", QSettings::NativeFormat).contains("DefaultShellAction");
}

unicorn::IPluginInfo::BootstrapType
unicorn::FooBar09PluginInfo::bootstrapType() const
{
    return NoBootstrap;
}

QString
unicorn::FooBar09PluginInfo::pluginInstaller() const
{
    return "FooPlugin0.9.4Setup_2.3.1.3.exe";
}

