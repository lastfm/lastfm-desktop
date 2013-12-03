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

#include <QBoxLayout>
#include <QLabel>

#include <lastfm/Library.h>
#include <lastfm/XmlQuery.h>

#include "lib/unicorn/widgets/Label.h"
#include "lib/unicorn/widgets/AvatarWidget.h"

#include "PlayableItemWidget.h"
#include "ProfileArtistWidget.h"
#include "ContextLabel.h"
#include "FriendWidget.h"

#include "../Services/ScrobbleService/ScrobbleService.h"
#include "../Application.h"

#include "ProfileWidget.h"

#include "ui_ProfileWidget.h"

ProfileWidget::ProfileWidget(QWidget *parent)
    :QFrame(parent), ui( new Ui::ProfileWidget )
{
    ui->setupUi( this );

    ui->scrobbles->setText( tr( "Scrobble(s)", "", 0 ) );
    ui->loved->setText( tr( "Loved track(s)", "", 0 ) );

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );
    connect( aApp, SIGNAL(gotUserInfo(lastfm::User)), SLOT(onGotUserInfo(lastfm::User)) );

    connect( &ScrobbleService::instance(), SIGNAL(scrobblesCached(QList<lastfm::Track>)), SLOT(onScrobblesCached(QList<lastfm::Track>)));

    onSessionChanged( aApp->currentSession() );
}

ProfileWidget::~ProfileWidget()
{
    delete ui;
}

void
ProfileWidget::onSessionChanged( const unicorn::Session& session )
{  
    if ( session.user().name() != m_currentUser )
    {
        m_currentUser = session.user().name();
        ui->avatar->setPixmap( QPixmap( ":/user_default.png" ) );
        onGotUserInfo( session.user() );

        refresh();
    }
}


void
ProfileWidget::onGotUserInfo( const lastfm::User& user )
{
     ui->avatar->setAlignment( Qt::AlignCenter );

     ui->avatar->setUser( user );
     ui->avatar->loadUrl( user.imageUrl( User::LargeImage, true ), HttpImageWidget::ScaleNone );
     ui->avatar->setHref( user.www() );

     ui->infoString->setText( FriendWidget::userString( user ) );

     ui->scrobbles->setText( tr( "Scrobble(s) since %1", "", user.scrobbleCount() ).arg( user.dateRegistered().toString( Qt::DefaultLocaleShortDate ) ) );

     m_scrobbleCount = user.scrobbleCount();
     setScrobbleCount();

     ui->name->setText( unicorn::Label::boldLinkStyle( unicorn::Label::anchor( user.www().toString(), user.name() ), Qt::black ) );
}

void
ProfileWidget::refresh()
{
    // Make sure we don't recieve any updates about the last session
    disconnect( this, SLOT(onGotLovedTracks()));
    disconnect( this, SLOT(onGotTopOverallArtists()));
    disconnect( this, SLOT(onGotTopWeeklyArtists()));
    disconnect( this, SLOT(onGotLibraryArtists()));

    connect( aApp->currentSession().user().getLovedTracks( 1 ), SIGNAL(finished()), SLOT(onGotLovedTracks()) );
    connect( aApp->currentSession().user().getTopArtists( "overall", 5, 1 ), SIGNAL(finished()), SLOT(onGotTopOverallArtists()));
    connect( aApp->currentSession().user().getTopArtists( "7day", 5, 1 ), SIGNAL(finished()), SLOT(onGotTopWeeklyArtists()));
    connect( lastfm::Library::getArtists( aApp->currentSession().user().name(), 1 ), SIGNAL(finished()), SLOT(onGotLibraryArtists()));
}

void
ProfileWidget::onCurrentChanged( int index )
{
    if ( index == 2 )
        refresh();
}

void
ProfileWidget::onGotLibraryArtists()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( static_cast<QNetworkReply*>(sender()) ) )
    {
        int scrobblesPerDay = aApp->currentSession().user().scrobbleCount() / (aApp->currentSession().user().dateRegistered().daysTo( QDateTime::currentDateTime() ) + 1 );
        int totalArtists = lfm["artists"].attribute( "total" ).toInt();

        QString artistsString = tr( "%L1 artist(s)", "", totalArtists ).arg( totalArtists );
        QString tracksString = tr( "%L1 track(s)", "", scrobblesPerDay ).arg( scrobblesPerDay );

        ui->userBlurb->setText( tr( "You have %1 in your library and on average listen to %2 per day." ).arg( artistsString , tracksString ) );
        ui->userBlurb->show();
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}


void
ProfileWidget::onGotTopWeeklyArtists()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {
        ui->weekFrame->setUpdatesEnabled( false );

        ui->weekFrame->layout()->takeAt( 0 )->widget()->deleteLater();

        QFrame* temp = new QFrame( this );
        ui->weekFrame->layout()->addWidget( temp );
        QVBoxLayout* layout = new QVBoxLayout( temp );
        layout->setContentsMargins( 0, 0, 0, 0 );
        layout->setSpacing( 0 );

        int maxPlays = lfm["topartists"]["artist"]["playcount"].text().toInt();

        foreach ( const lastfm::XmlQuery& artist, lfm["topartists"].children("artist") )
            layout->addWidget( new ProfileArtistWidget( artist, maxPlays, this ) );

        ui->weekFrame->setUpdatesEnabled( true );
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}


void
ProfileWidget::onGotTopOverallArtists()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {
        ui->overallFrame->setUpdatesEnabled( false );

        ui->overallFrame->layout()->takeAt( 0 )->widget()->deleteLater();

        QFrame* temp = new QFrame( this );
        ui->overallFrame->layout()->addWidget( temp );
        QVBoxLayout* layout = new QVBoxLayout( temp );
        layout->setContentsMargins( 0, 0, 0, 0 );
        layout->setSpacing( 0 );

        int maxPlays = lfm["topartists"]["artist"]["playcount"].text().toInt();

        foreach ( const lastfm::XmlQuery& artist, lfm["topartists"].children("artist") )
            layout->addWidget( new ProfileArtistWidget( artist, maxPlays, this ) );

        ui->overallFrame->setUpdatesEnabled( true );
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}

void
ProfileWidget::onGotLovedTracks()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {
        int lovedTrackCount = lfm["lovedtracks"].attribute( "total" ).toInt();
        ui->loved->setText( tr( "Loved track(s)", "", lovedTrackCount ) );
        ui->lovedCount->setText( QString( "%L1" ).arg( lovedTrackCount ) );
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}


void
ProfileWidget::onScrobblesCached( const QList<lastfm::Track>& tracks )
{
    foreach ( lastfm::Track track, tracks )
        connect( track.signalProxy(), SIGNAL(scrobbleStatusChanged( short )), SLOT(onScrobbleStatusChanged( short )));
}

void
ProfileWidget::onScrobbleStatusChanged( short scrobbleStatus )
{
    if (scrobbleStatus == lastfm::Track::Submitted)
    {
        disconnect( sender(), SIGNAL(scrobbleStatusChanged( short )), this, SLOT(onScrobbleStatusChanged( short )));
        ++m_scrobbleCount;
        setScrobbleCount();
    }
}

void
ProfileWidget::setScrobbleCount()
{
    ui->scrobbles->setText( tr( "Scrobble(s) since %1", "", aApp->currentSession().user().scrobbleCount() ).arg( aApp->currentSession().user().dateRegistered().toString( "d MMMM yyyy" ) ) );
    ui->scrobbleCount->setText( QString( "%L1" ).arg( m_scrobbleCount ) );
}
