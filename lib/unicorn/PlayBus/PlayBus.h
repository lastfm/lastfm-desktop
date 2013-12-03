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

#include <QLocalServer>
#include <QLocalSocket>
#include <QList>
#include <QString>
#include <QDir>
#include <QFile>
#include <QUuid>
#include <QCoreApplication>
#include <QDateTime>
#include <QSignalMapper>

#include "../SignalBlocker.h"

#ifdef Q_OS_WIN
#include <QSharedMemory>
#else
#include <lastfm/misc.h>
#endif

namespace unicorn
{

/** @author Jono Cole <jono@last.fm>
  * @brief An interprocess message bus.
  *
  * This is a client side implemented bus, meaning that 
  * if no bus is running then a client will become the master node.
  * 
  * If the master node drops it's connection then another client node
  * will become the master and all other clients will connect to this.
  *
  * This bus is designed to be run on a single host using local servers.
  * the server mediation is handled at the kernel level by exploiting the 
  * logic that allows only one process to listen on a named pipe.
  *
  * This code will not work across distributed hosts as no master node
  * mediation is carried out. This could be extended by using a known
  * mediation algorithm.
  */
class PlayBus : public QObject
{
Q_OBJECT
public:
    /** @param name The name of the bus to connect to or create */
    PlayBus( const QString& name, QObject* parent = 0 );
    ~PlayBus();

    void board();

    const QByteArray& lastMessage() const;

    void setQueryMessages( bool b );

public slots:
    QByteArray sendQuery( QByteArray request, int timeout = 200 );
    void sendQueryResponse( QString uuid, QByteArray message );

   /** send the message around the bus */
    void sendMessage( const QByteArray& msg );

signals:
    /** a new message has been received from the bus */
    void message( const QByteArray& msg );
    void queryRequest( const QString& uuid, const QByteArray& message);

private slots:
    void onSocketConnected();
    void reinit();
    void onError( const QLocalSocket::LocalSocketError& e );
    void onIncomingConnection();
    void processCommand( const QByteArray& data );
    void onSocketData();
    void onSocketDestroyed( QObject* o );

private:
    void addSocket( QLocalSocket* socket );

    const QStringList nodeList( const QString& data );

    QString m_busName;
    QLocalServer m_server;
    QList<QLocalSocket*> m_sockets;
    QByteArray m_lastMessage;
    QList<QString> m_dispatchedQueries;
    QList<QString> m_servicedQueries;
    QByteArray m_lastQueryResponse;
    QString m_lastQueryUuid;
    bool m_queryMessages;
#ifdef Q_OS_WIN
	QSharedMemory m_sharedMemory;
#endif
};

}

