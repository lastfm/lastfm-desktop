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

#include "Bus.h"

unicorn::Bus::Bus( QObject* parent )
    :PlayBus( "unicorn", parent )
{
    connect( this, SIGNAL( message(QByteArray)), SLOT( onMessage(QByteArray)));
    connect( this, SIGNAL( queryRequest( QString, QByteArray )), SLOT( onQuery( QString, QByteArray )));
}

bool
unicorn::Bus::isWizardRunning()
{
    return sendQuery( "WIZARDRUNNING" ) == "TRUE";
}

QMap<QString, QString>
unicorn::Bus::getSessionData()
{
    QByteArray ba = sendQuery( "SESSION" );
    QMap<QString, QString> sessionData;

    if( ba.length() > 0 )
    {
        QDataStream ds( ba );
        ds >> sessionData;
    }

    return sessionData;
}

void
unicorn::Bus::announceSessionChange( unicorn::Session& s )
{
    qDebug() << "Session change, let's spread the message through the bus!";
    QByteArray ba;
    QDataStream ds( &ba, QIODevice::WriteOnly | QIODevice::Truncate );

    ds << QString( "SESSIONCHANGED" );
    ds << ( s );

    sendMessage( ba );
}

void
unicorn::Bus::onMessage( const QByteArray& message )
{
    qDebug() << "Message received";
    qDebug() << "Message: " << message;
    QDataStream ds( message );
    QString stringMessage;

    ds >> stringMessage;

    qDebug() << stringMessage;

    if( stringMessage == "SESSIONCHANGED" )
    {
        qDebug() << "and it's a session change alert";
        unicorn::Session* session = new unicorn::Session( ds );
        emit sessionChanged( *session );
        delete session;
    }
    else if( message.startsWith( "LOVED=" ))
    {
        QByteArray sessionData = message.right( message.size() - 6);
        emit lovedStateChanged( sessionData == "true" );
    }
}

void
unicorn::Bus::onQuery( const QString& uuid, const QByteArray& message )
{
    qDebug() << "query received" << message;
    if( message == "WIZARDRUNNING" )
        emit wizardRunningQuery( uuid );
    else if( message == "SESSION" )
        emit sessionQuery( uuid );
}

