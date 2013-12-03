/***************************************************************************
 *   Copyright 2005-2008 Last.fm Ltd <client@last.fm>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#pragma once

#include "Version.h"

#include <lib/DllExportMacro.h>

#include <QObject>
#include <QString>

namespace unicorn
{

class UNICORN_DLLEXPORT ITunesPluginInstaller : public QObject
{
    Q_OBJECT
public:
    ITunesPluginInstaller( QWidget* parent );

    static Version installedVersion();
    static Version bundledVersion();
    
    void uninstall();

    // NOTE this is only valid after calling install()
    bool needsTwiddlyBootstrap() const { return m_needsTwiddlyBootstrap; }

public slots:
    void install();

private:
    bool isPluginInstalled();
    static Version pListVersion( const QString& file );

    bool removeInstalledPlugin();
    bool installPlugin();

    // Legacy code: removes old LastFmHelper for updates
    void disableLegacyHelperApp();

private:
    bool m_needsTwiddlyBootstrap;
};

}
