#ifndef PLAYBUS_H_
#define PLAYBUS_H_

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QList>
#include <QString>
#include <QtDebug>
#include <QDir>
#include <QFile>
#include <QUuid>
#include <QCoreApplication>
#include <QDateTime>
#include <QSignalMapper>
#include "SignalBlocker.h"
#ifdef Q_OS_WIN
	#include <QSharedMemory>
#else
    #include <lastfm/misc.h>
#endif



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
    PlayBus( const QString& name, QObject* parent = 0 )
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
        connect( &m_server, SIGNAL( newConnection()),
                            SLOT( onIncomingConnection()));
    }

    ~PlayBus()
    {
        m_server.close();
#ifdef Q_OS_WIN
		m_sharedMemory.detach();
#endif
    }

    void board()
    {
        reinit();
    }

    const QByteArray& lastMessage() const
    {
        return m_lastMessage;
    }

    void setQueryMessages( bool b )
    {
        m_queryMessages = b;
    }

public slots:
 
    QByteArray sendQuery( QByteArray request, int timeout = 200 )
    {
        QUuid quuid = QUuid::createUuid();
        QString uuid = quuid;
        m_dispatchedQueries << uuid;
        sendMessage( (uuid + " " + request).toUtf8() );

        SignalBlocker blocker( this, SIGNAL( queryRequest(QString,QByteArray)), timeout );
        
        while( blocker.start()) {
           if( m_lastQueryUuid == uuid ) {
                return m_lastQueryResponse;
            }
        }
        return QByteArray();
    }

    void sendQueryResponse( QString uuid, QByteArray message )
    {
        sendMessage( ( uuid + " " ).toUtf8() + message );
    }

   /** send the message around the bus */
    void sendMessage( const QByteArray& msg )
    {
        foreach( QLocalSocket* socket, m_sockets ) {
            socket->write( msg + "\n" );
            socket->flush();
        }

    }

signals:
    /** a new message has been received from the bus */
    void message( const QByteArray& msg );
    void queryRequest( const QString& uuid, const QByteArray& message);

private slots:

    void onSocketConnected()
    {
        //WIN32 supports GUIDs which almost certainly will be unique according to Qt.
        #ifndef WIN32
            //throw-away uuid generation to initialize random seed
            QUuid::createUuid();
            qsrand( (uint)QDateTime::currentDateTime().toTime_t() + QCoreApplication::applicationPid());
        #endif

        QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
        addSocket( socket );
    }

    void reinit()
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
		if( m_sharedMemory.create( 1 )) {
			emit message( "Now Listening" );
			m_server.listen( m_busName );
			return;
		} else
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

    void onError( const QLocalSocket::LocalSocketError& e )
    {
#ifdef Q_OS_WIN
        Q_UNUSED( e )
#endif

		if( e == QLocalSocket::ConnectionRefusedError ) {
			QFile::remove( m_busName );
		}
		QLocalSocket* s = qobject_cast<QLocalSocket*>(sender());
		s->close();
		s->deleteLater();
		QTimer::singleShot( 10, this, SLOT(reinit()));
    }

    void onIncomingConnection()
    {
        QLocalSocket* socket = 0;
        while( (socket = m_server.nextPendingConnection()) )
        {
			socket->setParent( this );
            addSocket( socket );
        }
    }

    void processCommand( const QByteArray& data )
    {
        m_lastMessage = data;
        QRegExp queryRE("^(\\{.{8}-.{4}-.{4}-.{4}-.{12}\\}) .*$");
        if( queryRE.indexIn( data ) > -1) {
            QString uuid = queryRE.cap(1);
            
            // ignore any queries that have already been seen
            if( !m_dispatchedQueries.contains( uuid ) && 
                 m_servicedQueries.contains( uuid )) {
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

    void onSocketData()
    {
        QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());

        while( socket->canReadLine()) {
            QByteArray data = socket->readLine();
            data.chop( 1 ); // remove trailing /n
            
            foreach( QLocalSocket* aSocket, m_sockets ) {
                if( aSocket == socket ) continue;
                aSocket->write( data + "\n" );
                aSocket->flush();
            }
            
            processCommand( data );

        }
    }
    
    void onSocketDestroyed( QObject* o )
    {
        QLocalSocket* s = dynamic_cast<QLocalSocket*>(o);
        if( !s ) return;
        s->blockSignals(true);
        m_sockets.removeAll( s );
    }

private:
    void addSocket( QLocalSocket* socket )
    {
        connect( socket, SIGNAL(readyRead()), SLOT(onSocketData()));
        QSignalMapper* mapper = new QSignalMapper(socket);
        mapper->setMapping( socket, socket );
        connect( mapper, SIGNAL(mapped(QObject*)), SLOT(onSocketDestroyed(QObject*)));
        connect( socket, SIGNAL(disconnected()), mapper, SLOT( map()));
        connect( socket, SIGNAL(destroyed(QObject*)), SLOT(onSocketDestroyed(QObject*)));
        m_sockets << socket;
    }

    const QStringList nodeList( const QString& data )
    {
        QString str( data );
        str = str.mid( 7 ); // remove NODES [
        str.chop( 1 ); // remove ]
        return str.split( "," );
    }

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


#endif //PLAYBUS_H_
