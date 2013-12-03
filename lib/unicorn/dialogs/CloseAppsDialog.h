/*
   Copyright 2012 Last.fm Ltd.
      - Primarily authored by Michael Coffey

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

#ifndef CLOSEAPPSDIALOG_H
#define CLOSEAPPSDIALOG_H

#include "lib/DllExportMacro.h"

#include <QDialog>

namespace Ui { class CloseAppsDialog; }

namespace unicorn
{
class IPluginInfo;

class UNICORN_DLLEXPORT CloseAppsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CloseAppsDialog( const QList<IPluginInfo*>& plugins, QWidget *parent );
    explicit CloseAppsDialog(QWidget *parent = 0);
    ~CloseAppsDialog();

    void setTitle( const QString& title );
    void setDescription( const QString& description );

    void showPluginList( bool showPluginList );

    void setOwnsPlugins( bool ownsPlugins );

    static bool isITunesRunning();

private:
#ifndef Q_OS_MAC
    static QStringList runningApps( const QList<IPluginInfo*>& plugins );
#else
    static // this method is only statis on mac
#endif
    QStringList runningApps();

    void showEvent( QShowEvent* event );

private slots:
    void checkApps();

private:
    void commonSetup();
    
private:
    Ui::CloseAppsDialog *ui;
    QList<IPluginInfo*> m_plugins;

    bool m_ownsPlugins;
};

}

#endif // CLOSEAPPSDIALOG_H
