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

#include "ITunesPluginInfo.h"

unicorn::ITunesPluginInfo::ITunesPluginInfo( QObject* parent )
    :IPluginInfo( parent )
{}

unicorn::Version
unicorn::ITunesPluginInfo::version() const
{
    return Version( 6, 0, 5, 4 );
}

QString
unicorn::ITunesPluginInfo::name() const
{
    return "iTunes";
}

QString
unicorn::ITunesPluginInfo::displayName() const
{
    return QString( "iTunes" );
}

QString
unicorn::ITunesPluginInfo::processName() const
{
    return QString( "iTunes.exe" );
}

QString
unicorn::ITunesPluginInfo::id() const
{
#ifdef Q_OS_WIN32
    return "itw";
#elif
    return "osx";
#endif
}

bool
unicorn::ITunesPluginInfo::isAppInstalled() const
{
#ifdef Q_OS_WIN32
    return QSettings("HKEY_CURRENT_USER\\Software\\Apple Computer, Inc.\\iTunes", QSettings::NativeFormat).contains("SM Shortcut Installed");
#elif
    return true;
#endif
}

unicorn::IPluginInfo::BootstrapType
unicorn::ITunesPluginInfo::bootstrapType() const
{
    return ClientBootstrap;
}

QString
unicorn::ITunesPluginInfo::pluginInstaller() const
{
    return "iTunesPluginWinSetup_6.0.5.4.exe";
}

