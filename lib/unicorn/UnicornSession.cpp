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

#include <QApplication>
#include <QDebug>
#include <QTimer>

#include <lastfm/User.h>
#include <lastfm/Auth.h>
#include <lastfm/XmlQuery.h>

#include "UnicornSession.h"
#include "UnicornSettings.h"

namespace unicorn {


QMap<QString, QString>
Session::lastSessionData()
{
    Settings s;
    QMap<QString, QString> sessionData;

    //use the Username setting or the first username if there have been any logged in previously
    QString username = s.value( "Username", QString() ).toString();

    if( !username.isEmpty() )
    {
        UserSettings us( username );

        sessionData[ "username" ] = username;
        const QString sk = us.value( "SessionKey", "" ).toString();

        if( !sk.isEmpty() )
            sessionData[ "sessionKey" ] = sk;
    }

    return sessionData;
}

Session::Session()
{
    m_info.valid = false;
}

Session::Session( const QString& username, QString sessionKey )
{
    init( username, sessionKey );
    connect( qApp, SIGNAL( internetConnectionUp() ), SLOT( fetchInfo() ) );
}

Session::Session( QDataStream& dataStream )
{
    dataStream >> *this;
    connect( qApp, SIGNAL( internetConnectionUp() ), SLOT( fetchInfo() ) );
}

QNetworkReply*
Session::getToken()
{
    QMap<QString, QString> params;
    params["method"] = "auth.getToken";
    return lastfm::ws::get( params );
}

QNetworkReply*
Session::getSession( QString token )
{
    QMap<QString, QString> params;
    params["method"] = "auth.getSession";
    params["token"] = token;
    return lastfm::ws::post( params, false );
}

QString
Session::subscriptionPriceString() const
{
    return m_info.subscriptionPrice;
}

bool
Session::isValid() const
{
    return m_info.valid;
}


bool
Session::youRadio() const
{
    return m_info.youRadio;
}

bool
Session::registeredRadio() const
{
    return m_info.registeredRadio;
}

bool
Session::subscriberRadio() const
{
    return m_info.subscriberRadio;
}

bool
Session::youWebRadio() const
{
    return m_info.youWebRadio;
}

bool
Session::registeredWebRadio() const
{
    return m_info.registeredWebRadio;
}

bool
Session::subscriberWebRadio() const
{
    return m_info.subscriberWebRadio;
}

QString 
Session::sessionKey() const
{
    return m_sessionKey;
}

lastfm::User
Session::user() const
{
    return m_user;
}

void 
Session::init( const QString& username, const QString& sessionKey )
{
    Settings s;
    s.setValue( "Username", username );

    m_user.setName( username );
    m_sessionKey = sessionKey;

    UserSettings us( username );
    m_user.setName( username );
    m_user.setScrobbleCount( us.scrobbleCount() );
    m_user.setDateRegistered( us.dateRegistered() );
    m_user.setRealName( us.realName() );
    m_user.setIsSubscriber( us.subscriber() );
    m_user.setType( us.type() );

    QList<QUrl> imageUrls;
    int imageCount = us.beginReadArray( "ImageUrls" );

    for ( int i = 0; i < imageCount; i++ )
    {
        us.setArrayIndex( i );
        imageUrls.append( us.value( "Url", QUrl() ).toUrl() );
    }

    us.endArray();

    m_user.setImages( imageUrls );

    if ( sessionKey.isEmpty() )
        Q_ASSERT( false );
    else
        us.setValue( "SessionKey", sessionKey );

    m_info = us.sessionInfo();

    fetchInfo();
}

void
Session::fetchInfo()
{
    qDebug() << "fetching user info";
    lastfm::ws::Username = m_user.name();
    lastfm::ws::SessionKey = m_sessionKey;
    connect( lastfm::User::getInfo(), SIGNAL( finished() ), SLOT( onUserGotInfo() ) );
    connect( lastfm::Auth::getSessionInfo(), SIGNAL(finished()), SLOT(onAuthGotSessionInfo()) );
}

void
Session::onUserGotInfo()
{
    QNetworkReply* reply = ( QNetworkReply* )sender();

    XmlQuery lfm;

    if ( lfm.parse( reply ) )
    {
        lastfm::User user( lfm["user"] );

        m_user = user;
        cacheUserInfo( m_user );

        emit userInfoUpdated( m_user );
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}

void
Session::onAuthGotSessionInfo()
{
    XmlQuery lfm;

    if ( lfm.parse( static_cast<QNetworkReply*>( sender() ) ) )
    {
        qDebug() << lfm;

        m_info.valid = true;

        m_info.subscriptionPrice = lfm["application"]["radioprice"]["formatted"].text();

        XmlQuery you = lfm["application"]["radioPermission"]["user type=you"];
        m_info.youRadio = you["radio"].text() == "1";
        m_info.youWebRadio = you["webradio"].text() == "1";

        XmlQuery registered = lfm["application"]["radioPermission"]["user type=registered"];
        m_info.registeredRadio = registered["radio"].text() == "1";
        m_info.registeredWebRadio = registered["webradio"].text() == "1";

        XmlQuery subscriber = lfm["application"]["radioPermission"]["user type=subscriber"];
        m_info.subscriberRadio = subscriber["radio"].text() == "1";
        m_info.subscriberWebRadio = subscriber["webradio"].text() == "1";

        bool isSubscriber = lfm["application"]["session"]["subscriber"].text() == "1";
        m_user.setIsSubscriber( isSubscriber );

        cacheUserInfo( m_user ); // make sure the subscriber flag gets cached
        cacheSessionInfo( *this );

        emit sessionChanged( *this );
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}

void
Session::cacheUserInfo( const lastfm::User& user )
{
    UserSettings us( user.name() );
    us.setSubscriber( user.isSubscriber() );
    us.setValue( "ScrobbleCount", user.scrobbleCount() );
    us.setValue( "DateRegistered", user.dateRegistered() );
    us.setValue( "RealName", user.realName() );
    us.setValue( "Type", user.type() );

    QList<User::ImageSize> sizes;
    sizes << User::SmallImage << User::MediumImage << User::LargeImage;

    us.beginWriteArray( "ImageUrls", sizes.count() );
    for ( int i = 0; i < sizes.count(); i++ )
    {
        us.setArrayIndex( i );
        us.setValue( "Url", user.imageUrl( sizes[ i ] ) );
    }
    us.endArray();
}

void
Session::cacheSessionInfo( const unicorn::Session& session )
{
    UserSettings userSettings( session.user().name() );
    userSettings.setSessionInfo( m_info );
}

QDataStream&
Session::write( QDataStream& out ) const
{
    QMap<QString, QString> data;
    data[ "username" ] = user().name();
    data[ "sessionkey" ] = m_sessionKey;
    out << data;
    return out;
}

QDataStream&
Session::read( QDataStream& in )
{
    QMap<QString, QString> data;
    in >> data;
    init( data[ "username" ], data[ "sessionkey" ] );
    return in;
}

}

QDataStream& operator<<( QDataStream& out, const unicorn::Session& s ){ return s.write( out ); }
QDataStream& operator>>( QDataStream& in, unicorn::Session& s ){ return s.read( in ); }

