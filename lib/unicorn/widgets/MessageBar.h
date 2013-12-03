/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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
#ifndef MESSAGE_BAR_H
#define MESSAGE_BAR_H

#include "lib/DllExportMacro.h"

#include <lastfm/Track.h>

#ifdef Q_OS_MAC
#include "lib/unicorn/plugins/ITunesPluginInstaller.h"
#elif defined( Q_OS_WIN )
#include "lib/unicorn/plugins/ITunesPluginInfo.h"
#endif

#include <QFrame>

class UNICORN_DLLEXPORT MessageBar : public QFrame
{
    Q_OBJECT 
private:
    struct
    {
        class QLabel* icon;
        class QLabel* message;
        class QPushButton* close;
    } ui;

public:
    explicit MessageBar( QWidget* parent );

    void addTracks( const QList<lastfm::Track>& tracks );
    const QList<lastfm::Track>& tracks() const;
    
public slots:
    void show( const QString& message, const QString& id = QString(), int timeout = -1 );

private slots:
    void onLinkActivated( const QString& link );
    void onCloseClicked();

private:
    QList<lastfm::Track> m_tracks;
    QPointer<QTimer> m_timeout;
#ifdef Q_OS_MAC
    QPointer<unicorn::ITunesPluginInstaller> m_installer;
#elif defined( Q_OS_WIN )
    QPointer<unicorn::ITunesPluginInfo> m_itw;
#endif
};

#endif
