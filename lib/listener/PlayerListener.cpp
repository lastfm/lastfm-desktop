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
#include "PlayerListener.h"
#include "PlayerCommandParser.h"
#include "PlayerConnection.h"
#include <QLocalSocket>
#include <QDir>
#include <QFile>
#include <QThread>

#ifdef Q_OS_WIN
#include "common/c++/win/scrobSubPipeName.cpp"
#include <tchar.h>
#include <strsafe.h>
#else
#include <lastfm/misc.h>
#endif

#define NUM_LISTENERS 8

PlayerListener::PlayerListener( QObject* parent ) throw( std::runtime_error )
              : QLocalServer( parent )
{
    connect( this, SIGNAL(newConnection()), SLOT(onNewConnection()) );

    // Create a user-unique name to listen on.
    // User-unique so that different logged-on users 
    // can run their own scrobbler instances.

#ifdef Q_OS_WIN
    connect( this, SIGNAL(pipeConnected()), this, SLOT(onPipeConnected()));

    m_connectEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    // have 8 listeners (for a laugh)
    for ( int i = 0; i < NUM_LISTENERS ; ++i )
    {
        if ( !addListener() )
            break;
    }

    HANDLE waitHandle;
    RegisterWaitForSingleObject( &waitHandle, m_connectEvent, PlayerListener::onConnectedNamedPipe, this, INFINITE, WT_EXECUTEONLYONCE );

#else
    QString const name = "lastfm_scrobsub";

    // on windows we use named pipes which auto-delete
    // *nix platforms need more help:

    QString fullPath = lastfm::dir::runtimeData().absolutePath() + "/" + name;
    if( QFile::exists( fullPath ))
        QFile::remove( fullPath );

    if (!listen( fullPath ))
        throw std::runtime_error( errorString().toStdString() );
#endif
}

#ifdef Q_OS_WIN
bool PlayerListener::addListener()
{
    // The object must not change its address once the
    // contained OVERLAPPED struct is passed to Windows.
    Listener listener;
    m_listeners << listener;

    qDebug() << QThread::currentThreadId();

    std::string s;
    DWORD r = scrobSubPipeName( &s );
    if (r != 0) throw std::runtime_error( formatWin32Error( r ) );
    QString const name = QString::fromStdString( s );

    m_listeners.last().pipeHandle = CreateNamedPipe(
                 (const wchar_t *)name.utf16(), // pipe name
                 PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,       // read/write access
                 PIPE_TYPE_MESSAGE |          // byte type pipe
                 PIPE_READMODE_MESSAGE |      // byte-read mode
                 PIPE_WAIT,                // blocking mode
                 PIPE_UNLIMITED_INSTANCES, // max. instances
                 BUFSIZE,                             // output buffer size
                 BUFSIZE,                             // input buffer size
                 3000,                     // client time-out
                 NULL);

    if ( m_listeners.last().pipeHandle == INVALID_HANDLE_VALUE )
    {
        qDebug() << "QLocalServerPrivate::addListener";
        m_listeners.removeLast();
        return false;
    }

    memset(&m_listeners.last().overlapped, 0, sizeof(m_listeners.last().overlapped));
    m_listeners.last().overlapped.hEvent = m_connectEvent;
    if (!ConnectNamedPipe(m_listeners.last().pipeHandle, &m_listeners.last().overlapped)) {
        switch (GetLastError()) {
        case ERROR_IO_PENDING:
            m_listeners.last().connected = false;
            break;
        case ERROR_PIPE_CONNECTED:
            m_listeners.last().connected = true;
            SetEvent(m_connectEvent);
            break;
        default:
            CloseHandle(m_listeners.last().pipeHandle);
            qDebug() << "QLocalServerPrivate::addListener ConnectNamedPipe";
            m_listeners.removeLast();
            return false;
        }
    } else {
        Q_ASSERT_X(false, "QLocalServerPrivate::addListener", "The impossible happened");
        SetEvent(m_connectEvent);
    }
    return true;
}

VOID
PlayerListener::onConnectedNamedPipe( PVOID lpParameter, BOOLEAN TimerOrWaitFired )
{
    PlayerListener* self = static_cast<PlayerListener*>(lpParameter);
    ResetEvent( self->m_connectEvent );
    emit self->pipeConnected();
}

void
PlayerListener::onPipeConnected()
{
    DWORD dummy;

    for ( int i = 0 ; i < m_listeners.length() ; )
    {
        HANDLE handle = m_listeners[i].pipeHandle;

        if ( m_listeners[i].connected || GetOverlappedResult( handle, &m_listeners[i].overlapped, &dummy, FALSE ) )
        {
            m_listeners.removeAt(i);

            addListener();

            // read from the pipe
            Listener listener;
            m_pipes << listener;
            m_pipes.last().pipeHandle = handle;
            m_pipes.last().self = this;

            memset(&listener.overlapped, 0, sizeof(listener.overlapped));
            BOOL fRead = ReadFileEx(
                            m_pipes.last().pipeHandle,
                            m_pipes.last().readBuffer,
                            BUFSIZE*sizeof(TCHAR),
                            &m_pipes.last().overlapped,
                            PlayerListener::onReadFileComplete);

            // Disconnect if an error occurred.
            if (!fRead)
            {
                qDebug() << "ReadFileEx error" << GetLastError();
                DisconnectNamedPipe( handle );
                CloseHandle( handle );
                m_pipes.removeLast();
            }
        }
        else
        {
            if ( GetLastError() != ERROR_IO_INCOMPLETE )
            {
                qDebug() << "an error with the pipes! ERROR_IO_INCOMPLETE";
                return;
            }

            ++i;
        }
    }
}

VOID
PlayerListener::onReadFileComplete( DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped )
{
    Listener* listener = (Listener*)(lpOverlapped);

    QByteArray response = listener->self->processLine( QString::fromUtf8( (char*)listener->readBuffer, dwNumberOfBytesTransfered ) ).toUtf8();

    StringCchCopy( listener->writeBuffer, response.size(), (LPCTSTR)response.data() );

    BOOL fWrite = WriteFileEx(
             listener->pipeHandle,
             listener->writeBuffer,
             response.size(),
             lpOverlapped,
             PlayerListener::onWriteFileComplete);

    if (!fWrite)
    {
        qDebug() << "WriteFileEx error" << GetLastError();
        DisconnectNamedPipe( listener->pipeHandle );
        CloseHandle( listener->pipeHandle );
    }

    // Register for the next callback here as we are now ready to recieve more data
    HANDLE waitHandle;
    RegisterWaitForSingleObject( &waitHandle, listener->self->m_connectEvent, PlayerListener::onConnectedNamedPipe, listener->self, INFINITE, WT_EXECUTEONLYONCE );
}

VOID
PlayerListener::onWriteFileComplete( DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped )
{
    Listener* listener = (Listener*)(lpOverlapped);

    for ( int i = 0 ; i < listener->self->m_pipes.count() ; ++i )
    {
        if ( listener->self->m_pipes[i].pipeHandle == listener->pipeHandle )
        {
            DisconnectNamedPipe( listener->pipeHandle );
            CloseHandle( listener->pipeHandle );
            listener->self->m_pipes.removeAt( i );
            break;
        }
    }
}

#endif

void
PlayerListener::onNewConnection()
{
    qDebug() << hasPendingConnections();

    while (hasPendingConnections())
    {
        QObject* o = nextPendingConnection();
        connect( o, SIGNAL(readyRead()), SLOT(onDataReady()) );
        connect( o, SIGNAL(disconnected()), o, SLOT(deleteLater()) );
    }
}

void
PlayerListener::onDataReady()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;

    while (socket->canReadLine())
    {
        QString const line = QString::fromUtf8( socket->readLine() );
        QString response = processLine( line );
        socket->write( response.toUtf8() );
    }
}

QString
PlayerListener::processLine( const QString& line )
{
    QString response = "OK\n";

    try
    {
        PlayerCommandParser parser( line );
        QString const id = parser.playerId();
        PlayerConnection* connection = 0;

        if (!m_connections.contains( id ))
        {
            connection = m_connections[id] = new PlayerConnection( parser.playerId(), parser.playerName() );
            emit newConnection( connection );
        }
        else
            connection = m_connections[id];

        switch (parser.command())
        {
            case CommandBootstrap:
                emit bootstrapCompleted( parser.playerId() );
                break;

            case CommandTerm:
                delete connection;
                m_connections.remove( parser.playerId() );
                break;

            default:
                connection->handleCommand( parser.command(), parser.track() );
                break;
        }
    }
    catch (std::invalid_argument& e)
    {
        qWarning() << e.what();
        response = "ERROR: " + QString::fromStdString(e.what()) + "\n";
    }

    return response;
}
