/*
   Copyright 2010-2012 Last.fm Ltd.
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

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QPointer>

#include "lib/DllExportMacro.h"

#ifdef Q_OS_WIN
namespace qtsparkle { class Updater; }
#define UPDATE_URL_WIN "http://cdn.last.fm/client/updates/updates.win.xml"
#define UPDATE_URL_WIN_BETA "http://cdn.last.fm/client/updates/updates.win.beta.xml"
#elif defined( Q_OS_MAC )
#define UPDATE_URL_MAC @"http://cdn.last.fm/client/updates/updates.mac.xml"
#define UPDATE_URL_MAC_BETA @"http://cdn.last.fm/client/updates/updates.mac.beta.xml"
#endif

namespace unicorn
{

class UNICORN_DLLEXPORT Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater( QWidget* parent = 0 );
    ~Updater();

    void setBetaUpdates( bool betaUpdates );

public slots:
    void checkForUpdates();

private:
#ifdef Q_OS_WIN
    QPointer<qtsparkle::Updater> m_updater;
    QWidget* m_parentWidget;
    bool m_betaUpdates;
#endif
};

}

#endif // UPDATER_H
