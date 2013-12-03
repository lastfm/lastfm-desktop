/*
   Copyright 2010 Last.fm Ltd.
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

#include <QApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QTimer>

#include <lastfm/Track.h>
#include <lastfm/UrlBuilder.h>

#include "lib/unicorn/dialogs/ShareDialog.h"
#include "lib/unicorn/dialogs/TagDialog.h"
#include "lib/unicorn/DesktopServices.h"

#include "../Services/ScrobbleService.h"
#include "../Application.h"

#include "RefreshButton.h"
#include "TrackWidget.h"
#include "ScrobblesListWidget.h"

#define kScrobbleLimit 30

class ScrobblesListWidgetItem : public QListWidgetItem
{
public:
    ScrobblesListWidgetItem( QListWidget* parent );
    bool operator<( const QListWidgetItem& that ) const;

    bool isNowPlaying() const;
    void setNowPlaying( bool nowPlaying );

    void setTrack( lastfm::Track& track );

private:
    bool m_nowPlaying;
};

ScrobblesListWidgetItem::ScrobblesListWidgetItem( QListWidget* parent )
    :QListWidgetItem( parent ), m_nowPlaying( false )
{
}

bool
ScrobblesListWidgetItem::isNowPlaying() const
{
    return m_nowPlaying;
}

void
ScrobblesListWidgetItem::setNowPlaying( bool nowPlaying )
{
    m_nowPlaying = nowPlaying;
    static_cast<TrackWidget*>( listWidget()->itemWidget( this ) )->setNowPlaying( true );
}

void
ScrobblesListWidgetItem::setTrack( lastfm::Track& track )
{
    static_cast<TrackWidget*>( listWidget()->itemWidget( this ) )->setTrack( track );
}

bool
ScrobblesListWidgetItem::operator<( const QListWidgetItem& that ) const
{
    if ( !qobject_cast<TrackWidget*>(listWidget()->itemWidget( const_cast<ScrobblesListWidgetItem*>(this) )) )
    {
        // this isn't a track widget

        if ( qobject_cast<RefreshButton*>(listWidget()->itemWidget( const_cast<ScrobblesListWidgetItem*>(this) )) )
            return true; // this is a refresh button
        else
            return false; // more button goes at the bottom
    }

    if ( qobject_cast<RefreshButton*>(listWidget()->itemWidget( const_cast<QListWidgetItem*>(&that) )) )
        return false; // that is a refresh button so this is not less than. refresh button goes at the top

    if ( !qobject_cast<TrackWidget*>(listWidget()->itemWidget( const_cast<QListWidgetItem*>(&that) )) )
        return true; // that is the more push button. everything else is less than it

    // at this point both this and that are of type TrackWidget

    // check if it's the now playing track
    if ( m_nowPlaying )
        return true; // now playing goes at the top of the tracks
    else if ( static_cast<const ScrobblesListWidgetItem*>(&that)->m_nowPlaying )
        return false;

    // now order by timestamp
    return static_cast<TrackWidget*>( listWidget()->itemWidget( const_cast<ScrobblesListWidgetItem*>( this ) ) )->track().timestamp().toTime_t()
            > static_cast<TrackWidget*>( listWidget()->itemWidget( const_cast<QListWidgetItem*>( &that ) ) )->track().timestamp().toTime_t();
}

ScrobblesListWidget::ScrobblesListWidget( QWidget* parent )
    :QListWidget( parent ), m_trackItem( 0 )
{
    setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef Q_OS_MAC
    connect( verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(scroll()) );
#endif

    setAttribute( Qt::WA_MacNoClickThrough );
    setAttribute( Qt::WA_MacShowFocusRect, false );

    setUniformItemSizes( false );
    setSortingEnabled( false );
    setSelectionMode( QAbstractItemView::NoSelection );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    connect( qApp, SIGNAL( sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)));

    connect( &ScrobbleService::instance(), SIGNAL(scrobblesCached(QList<lastfm::Track>)), SLOT(onScrobblesSubmitted(QList<lastfm::Track>) ) );
    connect( &ScrobbleService::instance(), SIGNAL(scrobblesSubmitted(QList<lastfm::Track>)), SLOT(onScrobblesSubmitted(QList<lastfm::Track>) ) );

    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), SLOT(onTrackStarted(lastfm::Track,lastfm::Track)));
    connect( &ScrobbleService::instance(), SIGNAL(paused()), SLOT(onPaused()));
    connect( &ScrobbleService::instance(), SIGNAL(resumed()), SLOT(onResumed()));
    connect( &ScrobbleService::instance(), SIGNAL(stopped()), SLOT(onStopped()));

    onSessionChanged( aApp->currentSession() );
}

#ifdef Q_OS_MAC
void
ScrobblesListWidget::scroll()
{
    // KLUDGE: The friend list widgets don't move unless we do this
    sortItems( Qt::AscendingOrder );
}
#endif

void
ScrobblesListWidget::showEvent(QShowEvent *)
{
    QList<lastfm::Track> tracks;

    for ( int i = 0 ; i < count() ; ++i )
    {
        TrackWidget* trackWidget = qobject_cast<TrackWidget*>( itemWidget( item( i ) ) );

        if ( trackWidget && !item( i )->isHidden() )
            tracks << trackWidget->track();
    }

    fetchTrackInfo( tracks );
}

void
ScrobblesListWidget::fetchTrackInfo( const QList<lastfm::Track>& tracks )
{
    if ( isVisible() )
    {
        // Make sure we fetch info for any tracks with unknown loved status
        foreach ( const lastfm::Track& track, tracks )
            if ( track.loveStatus() == lastfm::Track::UnknownLoveStatus )
                track.getInfo(  this, "write", User().name() );
    }
}

void
ScrobblesListWidget::mousePressEvent( QMouseEvent* event )
{
    event->setAccepted( false );
}

void
ScrobblesListWidget::mouseReleaseEvent( QMouseEvent* event )
{
    event->setAccepted( false );
}

void 
ScrobblesListWidget::onItemClicked( TrackWidget& trackWidget )
{
    emit trackClicked( trackWidget );
}

void
ScrobblesListWidget::onMoreClicked()
{
    unicorn::DesktopServices::openUrl( lastfm::UrlBuilder( "user" ).slash( User().name() ).slash( "tracks" ).url() );
}

void
ScrobblesListWidget::onSessionChanged( const unicorn::Session& session )
{
    if ( !session.user().name().isEmpty() )
    {
        QString path = lastfm::dir::runtimeData().filePath( session.user().name() + "_recent_tracks.xml" );

        if ( m_path != path )
        {
            m_path = path;
            read();
            refresh();
        }
    }
}


void
ScrobblesListWidget::read()
{
    clear();

    // always have a now playing item in the list
    m_trackItem = new ScrobblesListWidgetItem( this );
    TrackWidget* trackWidget = new TrackWidget( m_track, this );
    trackWidget->setObjectName( "nowPlaying" );
    setItemWidget( m_trackItem, trackWidget );
    m_trackItem->setSizeHint( trackWidget->sizeHint() );
    m_trackItem->setHidden( true );
    m_trackItem->setNowPlaying( true );

    connect( trackWidget, SIGNAL(clicked(TrackWidget&)), SLOT(onItemClicked(TrackWidget&)) );

    // always have the refresh button in the list
    m_refreshItem = new ScrobblesListWidgetItem( this );
    RefreshButton* refreshButton = new RefreshButton( this );
    refreshButton->setObjectName( "refresh" );
    setItemWidget( m_refreshItem, refreshButton );
    m_refreshItem->setSizeHint( refreshButton->sizeHint() );

    connect( refreshButton, SIGNAL(clicked()), SLOT(refresh()) );

    onRefreshing( false );

    // always have a view more item in the list
    m_moreItem = new ScrobblesListWidgetItem( this );
    QPushButton* moreButton = new QPushButton( tr( "More Scrobbles at Last.fm" ), this );
    moreButton->setObjectName( "more" );
    setItemWidget( m_moreItem, moreButton );
    m_moreItem->setSizeHint( moreButton->sizeHint() );

    connect( moreButton, SIGNAL(clicked()), SLOT(onMoreClicked()) );

    QFile file( m_path );
    file.open( QFile::Text | QFile::ReadOnly );
    QTextStream stream( &file );
    stream.setCodec( "UTF-8" );

    QDomDocument xml;
    xml.setContent( stream.readAll() );

    QList<Track> tracks;

    for (QDomNode n = xml.documentElement().lastChild(); !n.isNull(); n = n.previousSibling())
        tracks << Track( n.toElement() );

    addTracks( tracks );
    fetchTrackInfo( tracks );

    limit( kScrobbleLimit );
}

void
ScrobblesListWidget::write()
{
    if ( !m_writeTimer )
    {
        m_writeTimer = new QTimer( this );
        connect( m_writeTimer, SIGNAL(timeout()), this, SLOT(doWrite()) );
        m_writeTimer->setSingleShot( true );
    }

    m_writeTimer->start( 500 );
}

void
ScrobblesListWidget::doWrite()
{
    if ( count() == 0 )
        QFile::remove( m_path );
    else
    {
        QDomDocument xml;
        QDomElement e = xml.createElement( "recent_tracks" );
        e.setAttribute( "product", QCoreApplication::applicationName() );
        e.setAttribute( "version", "2" );

        for ( int i = 0 ; i < count() ; ++i )
        {
            TrackWidget* trackWidget = qobject_cast<TrackWidget*>( itemWidget( item( i ) ) );

            if ( trackWidget && !static_cast<ScrobblesListWidgetItem*>( item( i ) )->isNowPlaying() )
                e.appendChild( static_cast<TrackWidget*>( itemWidget( item( i ) ) )->track().toDomElement( xml ) );
        }

        xml.appendChild( e );

        QFile file( m_path );
        file.open( QIODevice::WriteOnly | QIODevice::Text );

        QTextStream stream( &file );
        stream.setCodec( "UTF-8" );
        stream << "<?xml version='1.0' encoding='utf-8'?>\n";
        stream << xml.toString( 2 );
    }
}

void
ScrobblesListWidget::onTrackStarted( const Track& track, const Track& )
{
    // Don't display Spotify here as we don't know if the current user is the one scrobbling
    // If it is the current user it will be fetch by user.getRecentTracks
    if ( track.extra( "playerId" ) != "spt" )
    {
        m_track = track;
        m_trackItem->setTrack( m_track );
        m_trackItem->setHidden( false );

        connect( m_track.signalProxy(), SIGNAL(loveToggled(bool)), SLOT(write()));

        QList<lastfm::Track> tracks;
        tracks << track;
        fetchTrackInfo( tracks );
    }

    hideScrobbledNowPlaying();
}

void
ScrobblesListWidget::onResumed()
{
    m_trackItem->setHidden( false );

    hideScrobbledNowPlaying();
}

void
ScrobblesListWidget::onPaused()
{
    m_trackItem->setHidden( true );

    hideScrobbledNowPlaying();
}

void
ScrobblesListWidget::onStopped()
{
    m_trackItem->setHidden( true );

    hideScrobbledNowPlaying();
}

void
ScrobblesListWidget::hideScrobbledNowPlaying()
{
    for ( int i = 0 ; i < count() ; ++i )
    {
        if ( item( i ) != m_trackItem )
        {
            TrackWidget* trackWidget = qobject_cast<TrackWidget*>( itemWidget( item( i ) ) );

            item( i )->setHidden( trackWidget
                                    && !m_trackItem->isHidden()
                                    && ( trackWidget->track().timestamp().toTime_t() == m_track.timestamp().toTime_t()
                                    || trackWidget->track().timestamp().toTime_t() == ScrobbleService::instance().currentTrack().timestamp().toTime_t() ) );
        }
    }
}

void
ScrobblesListWidget::refresh()
{
    if ( !m_recentTrackReply )
    {
        m_recentTrackReply = User().getRecentTracks( kScrobbleLimit, 1 );
        connect( m_recentTrackReply, SIGNAL(finished()), SLOT(onGotRecentTracks()) );
        onRefreshing( true );
    }
}


void
ScrobblesListWidget::onRefreshing( bool refreshing )
{
    RefreshButton* refreshButton = qobject_cast<RefreshButton*>( itemWidget( m_refreshItem ) );

    refreshButton->setText( refreshing ? tr( "Refreshing..." ) : tr( "Refresh Scrobbles" ) );
    refreshButton->setEnabled( !refreshing );
}

void
ScrobblesListWidget::onGotRecentTracks()
{
    XmlQuery lfm;

    if ( lfm.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {
        m_trackItem->setHidden( true );

        QList<lastfm::Track> tracks;
        lastfm::MutableTrack nowPlayingTrack;

        bool checkedFirstScrobble( false );

        foreach ( const XmlQuery& trackXml, lfm["recenttracks"].children("track") )
        {
            if ( trackXml.attribute( "nowplaying" ) == "true" )
            {
                nowPlayingTrack.setTitle( trackXml["name"].text() );
                nowPlayingTrack.setArtist( trackXml["artist"]["name"].text() );
                nowPlayingTrack.setAlbum( trackXml["album"].text() );

                if ( nowPlayingTrack != m_track )
                {
                    // This is a different track so change to it
                    nowPlayingTrack.setTimeStamp( QDateTime::fromTime_t( trackXml["date"].attribute("uts").toUInt() ) );

                    nowPlayingTrack.setImageUrl( Track::SmallImage, trackXml["image size=small"].text() );
                    nowPlayingTrack.setImageUrl( Track::MediumImage, trackXml["image size=medium"].text() );
                    nowPlayingTrack.setImageUrl( Track::LargeImage, trackXml["image size=large"].text() );
                    nowPlayingTrack.setImageUrl( Track::ExtraLargeImage, trackXml["image size=extralarge"].text() );

                    m_track = nowPlayingTrack;
                    m_trackItem->setTrack( m_track );

                    connect( m_track.signalProxy(), SIGNAL(loveToggled(bool)), SLOT(write()));

                    QString loved = trackXml["loved"].text();

                    if ( !loved.isEmpty() )
                        nowPlayingTrack.setLoved( loved == "1" );
                    else
                        m_track.getInfo( this, "write", User().name() );
                }

                m_trackItem->setHidden( false );
            }
            else
            {
                MutableTrack track;
                track.setTitle( trackXml["name"].text() );
                track.setArtist( trackXml["artist"]["name"].text() );
                track.setAlbum( trackXml["album"].text() );

                track.setTimeStamp( QDateTime::fromTime_t( trackXml["date"].attribute("uts").toUInt() ) );

                if ( checkedFirstScrobble ||
                      (!checkedFirstScrobble && ( track != nowPlayingTrack || (track == nowPlayingTrack && track.timestamp().secsTo( QDateTime::currentDateTime() ) > 10 * 60 ) ) ) )
                {
                    track.setImageUrl( Track::SmallImage, trackXml["image size=small"].text() );
                    track.setImageUrl( Track::MediumImage, trackXml["image size=medium"].text() );
                    track.setImageUrl( Track::LargeImage, trackXml["image size=large"].text() );
                    track.setImageUrl( Track::ExtraLargeImage, trackXml["image size=extralarge"].text() );

                    QString loved = trackXml["loved"].text();

                    if ( !loved.isEmpty() )
                        track.setLoved( loved == "1" );

                    tracks << track;
                }

                checkedFirstScrobble = true;
            }
        }

        QList<lastfm::Track> addedTracks = addTracks( tracks );

        // get info for track if we don't know the loved state. This is so it will
        // work before and after the loved field is added to user.getRecentTracks
        fetchTrackInfo( addedTracks );

        write();
    }

    onRefreshing( false );

    m_recentTrackReply->deleteLater();

    hideScrobbledNowPlaying();
}


void
ScrobblesListWidget::onScrobblesSubmitted( const QList<lastfm::Track>& tracks )
{
    // We need to find out if info has already been fetched for this track or not.
    // If the now playing view wasn't visible it won't have been.
    // Also, should also only fetch if the scrobbles list is visible too

    QList<lastfm::Track> addedTracks = addTracks( tracks );

    fetchTrackInfo( addedTracks );
}

void
ScrobblesListWidget::onTrackWidgetRemoved()
{
    for ( int i = 0 ; i < count() ; ++i )
    {
        if ( itemWidget( item( i ) ) == sender() )
        {
            itemWidget( takeItem( i ) )->deleteLater();
            refresh();
            break;
        }
    }
}

QList<lastfm::Track>
ScrobblesListWidget::addTracks( const QList<lastfm::Track>& tracks )
{
    QList<lastfm::Track> addedTracks;

    for ( int i = 0 ; i < tracks.count() ; ++i )
    {
        if ( tracks[i].scrobbleError() != Track::Invalid  )
        {
            // the track was not filtered client side for being invalid
            int pos = -1;

            for ( int j = 0 ; j < count() ; ++j )
            {
                TrackWidget* trackWidget = qobject_cast<TrackWidget*>( itemWidget( item( j ) ) );

                if ( trackWidget
                     && !static_cast<ScrobblesListWidgetItem*>( item( j ) )->isNowPlaying()
                     && tracks[i].timestamp().toTime_t() == trackWidget->track().timestamp().toTime_t() )
                {
                    pos = j;
                    break;
                }
            }

            if ( pos == -1 )
            {
                // the track was not in the list
                ScrobblesListWidgetItem* item = new ScrobblesListWidgetItem( this );
                Track track = tracks[i];
                TrackWidget* trackWidget = new TrackWidget( track, this );
                setItemWidget( item, trackWidget );
                item->setSizeHint( trackWidget->sizeHint() );

                connect( trackWidget, SIGNAL(removed()), SLOT(onTrackWidgetRemoved()));
                connect( trackWidget, SIGNAL(clicked(TrackWidget&)), SLOT(onItemClicked(TrackWidget&)) );

                connect( track.signalProxy(), SIGNAL(loveToggled(bool)), SLOT(write()));
                connect( track.signalProxy(), SIGNAL(scrobbleStatusChanged(short)), SLOT(write()));
            }
            else
            {
                // update the track in the list with the new infos!
                TrackWidget* trackWidget = qobject_cast<TrackWidget*>( itemWidget( item( pos ) ) );
                trackWidget->update( tracks[i] );
            }
        }
    }

    limit( kScrobbleLimit );

    write();

    hideScrobbledNowPlaying();

    return addedTracks;
}


void
ScrobblesListWidget::limit( int limit )
{
    sortItems();

    // we add three to the limit here to account for the refresh button,
    // the now playing track (usually hidden), and the more button
    if ( count() > limit + 3 )
    {
        while ( count() > limit + 3 )
        {
            QListWidgetItem* item = takeItem( count() - 2 );
            itemWidget( item )->deleteLater();
            delete item;
        }

        write();
    }
}

