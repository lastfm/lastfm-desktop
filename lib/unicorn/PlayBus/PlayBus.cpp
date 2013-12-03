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

#include "PlayBus.h"

unicorn::PlayBus::PlayBus( const QString& name, QObject* parent )
    :QObject( parent ),
     m_busName( name ),
     m_queryMessages( false )
#ifdef Q_OS_WIN
	 ,m_sharedMemory( name )
#endif
{
#ifndef Q_OS_WIN
    m_busName = lastfm::dir::runtimeData().absolutePath() + "/" + m_busName;
#endif
    connect( &m_server, SIGNAL( newConnection()), SLOT( onIncomingConnection()));
}

unicorn::PlayBus::~PlayBus()
{
    m_server.close();
#ifdef Q_OS_WIN
    m_sharedMemory.detach();
#endif
}

void
unicorn::PlayBus::board()
{
    reinit();
}

const QByteArray&
unicorn::PlayBus::lastMessage() const
{
    return m_lastMessage;
}

void
unicorn::PlayBus::setQueryMessages( bool b )
{
    m_queryMessages = b;
}

QByteArray
unicorn::PlayBus::sendQuery( QByteArray request, int timeout )
{
    QUuid quuid = QUuid::createUuid();
    QString uuid = quuid;
    m_dispatchedQueries << uuid;
    sendMessage( (uuid + " " + request).toUtf8() );

    SignalBlocker blocker( this, SIGNAL( queryRequest(QString,QByteArray)), timeout );

    while( blocker.start() )
        if( m_lastQueryUuid == uuid )
            return m_lastQueryResponse;

    return QByteArray();
}

void
unicorn::PlayBus::sendQueryResponse( QString uuid, QByteArray message )
{
    sendMessage( ( uuid + " " ).toUtf8() + message );
}

/** send the message around the bus */
void
unicorn::PlayBus::sendMessage( const QByteArray& msg )
{
    foreach( QLocalSocket* socket, m_sockets )
    {
        socket->write( msg + "\n" );
        socket->flush();
    }

}

void
unicorn::PlayBus::onSocketConnected()
{
    //WIN32 supports GUIDs which almost certainly will be unique according to Qt.
#ifndef WIN32
    //throw-away uuid generation to initialize random seed
    QUuid::createUuid();
#endif

    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    addSocket( socket );
}

void
unicorn::PlayBus::reinit()
{
    if( m_server.isListening())
        return;

    foreach( QLocalSocket* socket, m_sockets ) {
        m_sockets.removeAll(socket);
        socket->disconnect();
        socket->close();
        socket->deleteLater();
    }

#ifndef Q_OS_WIN
    if( m_server.listen( m_busName )) {
        return;
    }
#else
    if( m_sharedMemory.create( 1 ))
    {
        emit message( "Now Listening" );
        m_server.listen( m_busName );
        return;
    }
    else
    {
        emit message( "Connecting to server" );
    }
#endif

    m_server.close();
    QLocalSocket* socket = new QLocalSocket( this );
    connect( socket, SIGNAL( connected()), SLOT( onSocketConnected()));
    connect( socket, SIGNAL( disconnected()), SLOT( reinit()));
    connect( socket, SIGNAL( error(QLocalSocket::LocalSocketError)), SLOT( onError(QLocalSocket::LocalSocketError)));
    socket->connectToServer( m_busName );
}

void
unicorn::PlayBus::onError( const QLocalSocket::LocalSocketError& e )
{
#ifdef Q_OS_WIN
    Q_UNUSED( e )
#endif

    if( e == QLocalSocket::ConnectionRefusedError )
    {
        QFile::remove( m_busName );
    }

    QLocalSocket* s = qobject_cast<QLocalSocket*>(sender());
    s->close();
    s->deleteLater();
    QTimer::singleShot( 10, this, SLOT(reinit()));
}

void
unicorn::PlayBus::onIncomingConnection()
{
    QLocalSocket* socket = 0;
    while( (socket = m_server.nextPendingConnection()) )
    {
        socket->setParent( this );
        addSocket( socket );
    }
}

void
unicorn::PlayBus::processCommand( const QByteArray& data )
{
    m_lastMessage = data;
    QRegExp queryRE("^(\\{.{8}-.{4}-.{4}-.{4}-.{12}\\}) .*$");

    if( queryRE.indexIn( data ) > -1 )
    {
        QString uuid = queryRE.cap(1);

        // ignore any queries that have already been seen
        if( !m_dispatchedQueries.contains( uuid ) &&
             m_servicedQueries.contains( uuid ))
        {
             if( m_queryMessages )
                emit message( data );

             return;
        }

        m_lastQueryUuid = uuid;
        m_lastQueryResponse = data.mid( 39 ); //remove uuid and seperator
        m_servicedQueries << m_lastQueryUuid;

        emit queryRequest( m_lastQueryUuid, m_lastQueryResponse);

        if( !m_queryMessages )
            return;
    }

    emit message( data );
}

void
unicorn::PlayBus::onSocketData()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());

    while( socket->canReadLine() )
    {
        QByteArray data = socket->readLine();
        data.chop( 1 ); // remove trailing /n

        foreach( QLocalSocket* aSocket, m_sockets )
        {
            if( aSocket == socket )
                continue;

            aSocket->write( data + "\n" );
            aSocket->flush();
        }

        processCommand( data );

    }
}

void
unicorn::PlayBus::onSocketDestroyed( QObject* o )
{
    QLocalSocket* s = dynamic_cast<QLocalSocket*>(o);

    if( !s )
        return;

    s->blockSignals(true);
    m_sockets.removeAll( s );
}

void
unicorn::PlayBus::addSocket( QLocalSocket* socket )
{
    connect( socket, SIGNAL(readyRead()), SLOT(onSocketData()));
    QSignalMapper* mapper = new QSignalMapper(socket);
    mapper->setMapping( socket, socket );
    connect( mapper, SIGNAL(mapped(QObject*)), SLOT(onSocketDestroyed(QObject*)));
    connect( socket, SIGNAL(disconnected()), mapper, SLOT( map()));
    connect( socket, SIGNAL(destroyed(QObject*)), SLOT(onSocketDestroyed(QObject*)));
    m_sockets << socket;
}

const QStringList
unicorn::PlayBus::nodeList( const QString& data )
{
    QString str( data );
    str = str.mid( 7 ); // remove NODES [
    str.chop( 1 ); // remove ]
    return str.split( "," );
}

