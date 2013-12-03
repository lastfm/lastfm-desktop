/*
   Copyright 2011 Last.fm Ltd.
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

#include <QHBoxLayout>
#include <QMovie>
#include <QTimer>

#include <lastfm/RadioStation.h>

#include "lib/unicorn/widgets/AvatarWidget.h"
#include "lib/unicorn/widgets/Label.h"

#include "../Application.h"
#include "PlayableItemWidget.h"

#include "FriendWidget.h"
#include "ui_FriendWidget.h"



FriendWidget::FriendWidget( const lastfm::XmlQuery& user, QWidget* parent)
    :QFrame( parent ),
      ui( new Ui::FriendWidget ),
      m_user( user ),
      m_order( 0 - 1 ),
      m_listeningNow( false )
{   
    ui->setupUi( this );

    layout()->setAlignment( ui->avatar, Qt::AlignTop );

    m_movie = new QMovie( ":/icon_eq.gif", "GIF", this );
    m_movie->setCacheMode( QMovie::CacheAll );
    ui->equaliser->setMovie( m_movie );
    ui->equaliser->hide();

    update( user, -1 );

    QRegExp re( "/serve/(\\d*)s?/" );
    ui->avatar->loadUrl( user["image size=medium"].text().replace( re, "/serve/\\1s/" ), HttpImageWidget::ScaleNone );
    ui->avatar->setHref( user["url"].text() );

    ui->radio->setStation( RadioStation::library( User( user["name"].text() ) ), tr("%1's Library Radio").arg( user["name"].text() ), "" );

    ui->avatar->setUser( m_user );
}

void
FriendWidget::update( const lastfm::XmlQuery& user, unsigned int order )
{
    m_order = order;

    m_track.setTitle( user["recenttrack"]["name"].text() );
    m_track.setAlbum( user["recenttrack"]["album"]["name"].text() );
    m_track.setArtist( user["recenttrack"]["artist"]["name"].text() );
    m_track.setExtra( "playerName", user["scrobblesource"]["name"].text() );
    m_track.setExtra( "playerURL", user["scrobblesource"]["url"].text() );

    QString recentTrackDate = user["recenttrack"].attribute( "uts" );

    bool hasListened = m_track != lastfm::Track();
    ui->trackFrame->setVisible( hasListened );

    m_listeningNow = recentTrackDate.isEmpty() && hasListened;

    if ( !recentTrackDate.isEmpty() )
        m_track.setTimeStamp( QDateTime::fromTime_t( recentTrackDate.toUInt() ) );

    setDetails();
}

void
FriendWidget::setOrder( int order )
{
    m_order = order;
}

void
FriendWidget::setListeningNow( bool listeningNow )
{
    m_listeningNow = listeningNow;
}

bool
FriendWidget::operator<( const FriendWidget& that ) const
{
    // sort by most recently listened and then by name

    if ( this->m_listeningNow && !that.m_listeningNow )
        return true;

    if ( !this->m_listeningNow && that.m_listeningNow )
        return false;

    if ( this->m_listeningNow && that.m_listeningNow )
        return this->name().toLower() < that.name().toLower();

    if ( !this->m_track.timestamp().isNull() && that.m_track.timestamp().isNull() )
        return true;

    if ( this->m_track.timestamp().isNull() && !that.m_track.timestamp().isNull() )
        return false;

    if ( this->m_track.timestamp().isNull() && that.m_track.timestamp().isNull() )
        return this->name().toLower() < that.name().toLower();

    // both timestamps are valid!

    if ( this->m_track.timestamp() == that.m_track.timestamp() )
    {
        if ( this->m_order == that.m_order )
            return this->name().toLower() < that.name().toLower();

        return this->m_order < that.m_order;
    }

    // this is the other way around because a higher time means it's lower in the list
    return this->m_track.timestamp() > that.m_track.timestamp();
}

QString
FriendWidget::genderString( const lastfm::Gender& gender )
{
    QString result;

    if ( gender.male() )
        result = tr( "Male" );
    else if ( gender.female() )
        result = tr( "Female" );
    else
        result = tr( "Neuter" );

    return result;
}

QString
FriendWidget::userString( const lastfm::User& user )
{
    QString text;

    text = QString("%1").arg( user.realName().isEmpty() ? user.name() : user.realName() );
    if ( user.age() ) text.append( QString(", %1").arg( user.age() ) );
    if ( user.gender().known() ) text.append( QString(", %1").arg( genderString( user.gender() ) ) );
    if ( !user.country().isEmpty() ) text.append( QString(", %1").arg( user.country() ) );

    return text;
}

void
FriendWidget::setDetails()
{
    ui->userDetails->setText( userString( m_user ) );
    ui->username->setText( Label::boldLinkStyle( Label::anchor( m_user.www().toString(), name() ), Qt::black ) );
    ui->lastTrack->setText( m_track.toString() );

    if ( m_listeningNow )
    {
        // show the
        m_movie->start();
        ui->equaliser->show();

        ui->trackFrame->setObjectName( "nowListening" );
        style()->polish( ui->trackFrame );

        if ( !m_track.extra( "playerName" ).isEmpty() )
            ui->timestamp->setText( tr( "Scrobbling now from %1" ).arg( m_track.extra( "playerName" ) ) );
        else
            ui->timestamp->setText( tr( "Scrobbling now" ) );

        if ( m_timestampTimer ) m_timestampTimer->stop();
    }
    else
    {
        m_movie->stop();
        ui->equaliser->hide();

        ui->trackFrame->setObjectName( "groupBox" );
        style()->polish( ui->trackFrame );

        updateTimestamp();
    }
}

void
FriendWidget::updateTimestamp()
{
    if ( !m_timestampTimer )
    {
        m_timestampTimer = new QTimer( this );
        m_timestampTimer->setSingleShot( true );
        connect( m_timestampTimer, SIGNAL(timeout()), SLOT(updateTimestamp()));
    }

    unicorn::Label::prettyTime( *ui->timestamp, m_track.timestamp(), m_timestampTimer );
}

QString
FriendWidget::name() const
{
    return m_user.name();
}

QString
FriendWidget::realname() const
{
    return m_user.realName();
}
