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

#pragma once

#include "../UnicornSession.h"
#include "PlayBus.h"

namespace unicorn
{

class Bus : public PlayBus
{
    Q_OBJECT
public:
    Bus( QObject* parent = 0 );

    bool isWizardRunning();
    QMap<QString, QString> getSessionData();
    void announceSessionChange( unicorn::Session& s );

private slots:
    void onMessage( const QByteArray& message );
    void onQuery( const QString& uuid, const QByteArray& message );

signals:
    void wizardRunningQuery( const QString& uuid );
    void sessionQuery( const QString& uuid );
    void sessionChanged( const unicorn::Session& s );
    void rosterUpdated();
    void lovedStateChanged(bool loved);
};

}
