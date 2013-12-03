/*
   Copyright 2009-2012 Last.fm Ltd.
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

#ifndef UNICORN_SESSION_H_
#define UNICORN_SESSION_H_

#include <lastfm/XmlQuery.h>
#include <lastfm/misc.h>
#include <lastfm/User.h>
#include <lastfm/ws.h>

#include <QObject>
#include <QSharedData>

#include "lib/DllExportMacro.h"

namespace unicorn {

class UNICORN_DLLEXPORT Session : public QObject
{
    Q_OBJECT
public:
    struct Info
    {
        bool valid;

        QString subscriptionPrice;

        bool youRadio;
        bool registeredRadio;
        bool subscriberRadio;

        bool youWebRadio;
        bool registeredWebRadio;
        bool subscriberWebRadio;
    };

public:
    /** Return session object from stored session */
    Session();
    Session( QDataStream& dataStream );
    Session( const QString& username, QString sessionKey = "" );

    bool isValid() const;

    // client radio permissions
    bool youRadio() const;
    bool registeredRadio() const;
    bool subscriberRadio() const;

    // web radio permissions
    bool youWebRadio() const;
    bool registeredWebRadio() const;
    bool subscriberWebRadio() const;

    QString subscriptionPriceString() const;

    QString sessionKey() const;
    lastfm::User user() const;

    static QNetworkReply* getToken();
    static QNetworkReply* getSession( QString token );
    static QMap<QString, QString> lastSessionData();

    QDataStream& write( QDataStream& out ) const;
    QDataStream& read( QDataStream& in );

signals:
    void userInfoUpdated( const lastfm::User& user );
    void sessionChanged( const unicorn::Session& session );

protected:
    void init( const QString& username, const QString& sessionKey );

private:
    void cacheUserInfo( const lastfm::User& user );
    void cacheSessionInfo( const unicorn::Session& session );

private slots:
    void fetchInfo();
    void onUserGotInfo();
    void onAuthGotSessionInfo();

private:
    QString m_prevUsername;
    QString m_sessionKey;
    lastfm::User m_user;

    Info m_info;
};

}

QDataStream& operator<<( QDataStream& out, const unicorn::Session& s );
QDataStream& operator>>( QDataStream& in, unicorn::Session& s );

#endif //UNICORN_SESSION_H_
