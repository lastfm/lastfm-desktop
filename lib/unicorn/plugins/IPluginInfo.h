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

#ifndef PLUGIN_INFO_H_
#define PLUGIN_INFO_H_

#include "lib/unicorn/plugins/Version.h"

#include <lib/DllExportMacro.h>

#include <QString>
#include <QSettings>

#ifdef WIN32
#include <windows.h>
#include <Shlobj.h>
#endif

namespace unicorn
{

class UNICORN_DLLEXPORT IPluginInfo : public QObject
{
    Q_OBJECT
public:
    IPluginInfo( QObject* parent = 0);

    enum BootstrapType{ NoBootstrap = 0, ClientBootstrap, PluginBootstrap };

    bool install() const;


    virtual QString name() const = 0;

    virtual Version version() const = 0;
    Version installedVersion() const;

    virtual QString id() const = 0;
    virtual BootstrapType bootstrapType() const = 0;

    virtual bool isAppInstalled() const = 0;

    virtual QString processName() const = 0;

    // DisplayName string value as found in the HKEY_LM/Software/Microsoft/CurrentVersion/Uninstall/{GUID}/
    virtual QString displayName() const = 0;

    virtual QString pluginInstaller() const = 0;

#ifdef Q_OS_WIN
    static BOOL isWow64();
#endif

    virtual bool isInstalled() const;
    bool canBootstrap() const;

    void setVerbose( bool verbose );

public slots:
    bool doInstall();
    void install( bool install );

protected:
    QString programFilesX86() const;
    QString programFiles64() const;

private:
    bool m_install;
    bool m_verbose;
};

}
#endif //PLUGIN_INFO_H_
