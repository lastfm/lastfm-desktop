/*
   Copyright (C) 2013 John Stamp <jstamp@mehercule.net>

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

#ifndef MPRIS2LISTENER_H
#define MPRIS2LISTENER_H

#include <QHash>
#include <QPointer>
#include <lastfm/Track.h>

class Mpris2Service;
class Mpris2Connection;
class PlayerConnection;

class Mpris2Listener : public QObject
{
    Q_OBJECT
public:
    Mpris2Listener( QObject * parent );
    ~Mpris2Listener();
    void createConnection();

signals:
    void newConnection( PlayerConnection* );

private:
    QHash<QString, Mpris2Service*> m_services;
    QString m_currentService;
    QPointer<Mpris2Connection> m_connection;
    lastfm::Track m_lastTrack;
    QString m_lastPlayerState;

    void addService( const QString& name );
    void removeService( const QString& name );
    void stop();

private slots:
    void onServiceOwnerChanged( const QString& name,
            const QString& oldOwner,
            const QString& newOwner );
    void onChangedState( const QString& state );
};

#endif
