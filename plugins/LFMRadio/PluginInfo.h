/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Jono Cole

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

#ifndef LFM_PLUGIN_INFO_H_
#define LFM_PLUGIN_INFO_H_

#include "../../lib/unicorn/Updater/IPluginInfo.h"
#include "../../lib/DllExportMacro.h"

class UNICORN_DLLEXPORT LFMRadioPluginInfo : public IPluginInfo
{
public:
    std::string name() const { return "Last.fm Radio"; }
    Version minVersion() const { return Version(); }
    Version maxVersion() const { return Version(); }
    
    std::string pluginPath() const { return std::string( "" ); }
    std::string displayName() const { return std::string( "" ); }
    std::string processName() const { return std::string( "radio.exe" ); }

    std::string id() const { return "ass"; }
    BootstrapType bootstrapType() const { return NoBootstrap; }

    bool isPlatformSupported() const
    {
        return true;
    }

    bool isAppInstalled() const { return true; }
    bool isInstalled() const { return true; }

    #ifdef QT_VERSION
    std::tstring pluginInstallPath() const
    {
        return std::tstring();
    }
    #endif
 
};

#endif //LFM_PLUGIN_INFO_H_

