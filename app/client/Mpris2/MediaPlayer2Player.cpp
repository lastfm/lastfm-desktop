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

#include "MediaPlayer2Player.h"
#include "Application.h"

#include <lastfm/AbstractType.h>
#include <lastfm/User.h>
#include <phonon/mediaobject.h>

#include "../Services/RadioService.h"
#include "lib/unicorn/UnicornSettings.h"

MediaPlayer2Player::MediaPlayer2Player( QObject* parent ) : DBusAbstractAdaptor( parent )
{
    m_playbackState = Stopped;
    connect( &RadioService::instance(), SIGNAL(paused()),  SLOT(onPlaybackStateChanged()) );
    connect( &RadioService::instance(), SIGNAL(buffering(int)),  SLOT(onPlaybackStateChanged()) );
    connect( &RadioService::instance(), SIGNAL(stopped()), SLOT(onPlaybackStateChanged()) );
    connect( &RadioService::instance(), SIGNAL(resumed()), SLOT(onPlaybackStateChanged()) );
    connect( &RadioService::instance(), SIGNAL(trackSpooled( const Track& )), SLOT(onTrackChanged( const Track& )) );
    connect( &aApp->currentSession(), SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionInfo()) );
    if ( RadioService::instance().audioOutput() )
    {
        connect( RadioService::instance().audioOutput(), SIGNAL(volumeChanged( qreal )), SLOT(onVolumeChanged( qreal )) );
    }
}


// Properties

QString
MediaPlayer2Player::PlaybackStatus() const
{
    switch (m_playbackState)
    {
        case Playing:
            return "Playing";
        case Paused:
            return "Paused";
        default:
            return "Stopped";
    }
}


double
MediaPlayer2Player::Rate() const
{
    return 1.0;
}


void
MediaPlayer2Player::SetRate( double )
{
}


QVariantMap
MediaPlayer2Player::Metadata() const
{
    QVariantMap metaData;
    if ( m_track.isNull() )
        return metaData;

    metaData["mpris:trackid"] = QVariant::fromValue<QDBusObjectPath>(
                                    QDBusObjectPath( "/fm/last/scrobbler/" + QString::number( m_track.timestamp().toTime_t() ) ) );
    metaData["mpris:length"] = static_cast<qint64>(m_track.duration() * 1000000);
    if ( !m_track.imageUrl( AbstractType::LargeImage, true ).toString().isEmpty() )
        metaData["mpris:artUrl"] = m_track.imageUrl( AbstractType::LargeImage, true ).toString();
    if ( !m_track.album().title().isEmpty() )
        metaData["xesam:album"] = m_track.album().title();
    if ( ! m_track.albumArtist().name().isEmpty() )
        metaData["xesam:albumArtist"] = QStringList() << m_track.albumArtist().name();
    metaData["xesam:artist"] = QStringList() << m_track.artist().name();
    // xesam:asText
    // xesam:audioBPM
    // xesam:autoRating
    // xesam:comment
    // xesam:composer
    // xesam:contentCreated
    // xesam:discNumber
    // xesam:firstUsed
    // xesam:genre
    // xesam:lastUsed
    // xesam:lyricist
    metaData["xesam:title"] = m_track.title();
    if ( m_track.trackNumber() > 0 )
        metaData["xesam:trackNumber"] = m_track.trackNumber();
    metaData["xesam:url"] = m_track.url().toString();
    // xesam:useCount
    // xesam:userRating

    return metaData;
}


double
MediaPlayer2Player::Volume() const
{
    if ( RadioService::instance().audioOutput() )
        return RadioService::instance().audioOutput()->volume();
    else
        return 0;
}


void
MediaPlayer2Player::SetVolume( double vol )
{
    if ( vol < 0.0 )
        vol = 0.0;
    else if ( vol > 1.0 )
        vol = 1.0;
    if ( RadioService::instance().audioOutput() )
        RadioService::instance().audioOutput()->setVolume( vol );
}


qlonglong
MediaPlayer2Player::Position() const
{
    if ( RadioService::instance().mediaObject() )
        return RadioService::instance().mediaObject()->currentTime() * 1000;
    else
        return 0;
}


double
MediaPlayer2Player::MinimumRate() const
{
    return 1.0;
}


double
MediaPlayer2Player::MaximumRate() const
{
    return 1.0;
}


bool
MediaPlayer2Player::CanGoNext() const
{
    if ( m_playbackState == Playing || m_playbackState == Paused )
        return true;
    else
        return false;
}


bool
MediaPlayer2Player::CanGoPrevious() const
{
    return false;
}


bool
MediaPlayer2Player::CanPlay() const
{
    if ( !aApp->currentSession().youRadio() )
        return false;
    else
        return true;
}


bool
MediaPlayer2Player::CanPause() const
{
    if ( m_playbackState == Playing || m_playbackState == Paused )
        return true;
    else
        return false;
}


bool
MediaPlayer2Player::CanSeek() const
{
    return false;
}


bool
MediaPlayer2Player::CanControl() const
{
    return true;
}


// Methods

void
MediaPlayer2Player::Next() const
{
    RadioService::instance().skip();
}


void
MediaPlayer2Player::Previous() const
{
}


void
MediaPlayer2Player::Pause() const
{
    RadioService::instance().pause();
}


void
MediaPlayer2Player::PlayPause() const
{
    if ( RadioService::instance().state() == Playing )
        RadioService::instance().pause();
    else if ( !RadioService::instance().state() != Buffering )
        Play();
}


void
MediaPlayer2Player::Stop() const
{
    RadioService::instance().stop();
}


void
MediaPlayer2Player::Play() const
{
    if ( RadioService::instance().state() == Stopped )
        RadioService::instance().play( RadioStation( "" ) );
    else if ( !RadioService::instance().state() != Buffering )
        RadioService::instance().resume();
}


void
MediaPlayer2Player::Seek( qlonglong ) const
{
}


void
MediaPlayer2Player::SetPosition( const QDBusObjectPath&, qlonglong ) const
{
}


void
MediaPlayer2Player::OpenUri( QString uri ) const
{
    RadioService::instance().play( uri );
}


// Private slots

void
MediaPlayer2Player::onTrackChanged( const Track& track )
{
    if ( m_playbackState != Stopped )
    {
        m_playbackState = Stopped;
        signalPropertyChange( "PlaybackStatus", PlaybackStatus() );
        signalPropertyChange( "CanPause", CanPause() );
    }

    m_track = track;
    signalPropertyChange( "Metadata", Metadata() );
}


void
MediaPlayer2Player::onVolumeChanged( qreal vol )
{
    signalPropertyChange( "Volume", Volume() );
}


void
MediaPlayer2Player::onPlaybackStateChanged()
{
    State teststate = RadioService::instance().state();
    switch ( teststate )
    {
        case Playing:
        case Paused:
            break;
        case Buffering:
            teststate = Paused;
            break;
        default:
            teststate = Stopped;
    }
    if ( teststate != m_playbackState )
    {
        m_playbackState = teststate;
        if ( m_playbackState == Stopped )
        {
            m_track = Track();
            signalPropertyChange( "Metadata", Metadata() );
        }
        signalPropertyChange( "PlaybackStatus", PlaybackStatus() );
        signalPropertyChange( "CanPause", CanPause() );
        signalPropertyChange( "CanGoNext", CanGoNext() );
    }
}


void
MediaPlayer2Player::onSessionInfo()
{
    signalPropertyChange( "CanPlay", CanPlay() );
    signalPropertyChange( "CanPause", CanPause() );
}
