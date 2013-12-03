/*
   Copyright 2011-2012 Last.fm Ltd.
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

#include <QColor>
#include <QDebug>

#include <lastfm/ScrobbleCache.h>

#include "ScrobblesModel.h"

ScrobblesModel::Scrobble::Scrobble( const lastfm::Track track )
    :m_track( track ), m_scrobblingEnabled( true )
{
    bool ok;
    int count = m_track.extra( "playCount" ).toInt( &ok );
    m_originalPlayCount = ok ? count : 1;
}

lastfm::Track
ScrobblesModel::Scrobble::track() const
{
    return m_track;
}

QString
ScrobblesModel::Scrobble::title() const
{
    return m_track.title();
}

QString
ScrobblesModel::Scrobble::artist() const
{
    return m_track.artist();
}

QString
ScrobblesModel::Scrobble::album() const
{
    return m_track.album();
}

QDateTime
ScrobblesModel::Scrobble::timestamp() const
{
    return m_track.timestamp();
}

bool
ScrobblesModel::Scrobble::isLoved() const
{
    return m_track.isLoved();
}

bool
ScrobblesModel::Scrobble::isScrobblingEnabled() const
{
    return m_scrobblingEnabled;
}

void ScrobblesModel::Scrobble::setEnableScrobbling( bool allow )
{
    m_scrobblingEnabled = allow;
}

QVariant
ScrobblesModel::Scrobble::attribute( int index ) const
{
    switch( index )
    {
        case ScrobblesModel::Artist: return artist();
        case ScrobblesModel::Title: return title();
        case ScrobblesModel::Album: return album();
        case ScrobblesModel::TimeStamp: return timestamp();
        case ScrobblesModel::Plays: return track().extra( "playCount" ).toInt();
        case ScrobblesModel::Loved: return isLoved();
        default: break;
    }

    return QVariant();
}

int
ScrobblesModel::Scrobble::originalPlayCount() const
{
    return m_originalPlayCount;
}

bool
ScrobblesModel::Scrobble::operator<( const Scrobble& that ) const
{
    return this->m_track < that.m_track;
}


ScrobblesModel::ScrobblesModel( QObject* parent )
    : QAbstractTableModel( parent ), m_readOnly( false )
{
    m_headerTitles.append( tr( "Artist" ) );
    m_headerTitles.append( tr( "Title" ) );
    m_headerTitles.append( tr( "Album" ) );
    m_headerTitles.append( tr( "Plays" ) );
    m_headerTitles.append( tr( "Last Played" ) );
    m_headerTitles.append( tr( "Loved" ) );
}

void
ScrobblesModel::addTracks( const QList<lastfm::Track>& tracks )
{
    beginInsertRows( QModelIndex(), m_scrobbleList.count(), m_scrobbleList.count() + tracks.count() - 1 );

    foreach( lastfm::Track t, tracks )
        m_scrobbleList.append( t );

    endInsertRows();
}

void
ScrobblesModel::setReadOnly()
{
    m_readOnly = true;
}

int
ScrobblesModel::rowCount( const QModelIndex& /*parent*/ ) const
{
    return m_scrobbleList.count();
}

int
ScrobblesModel::columnCount( const QModelIndex& /*parent*/ ) const
{
    return m_headerTitles.count();
}

QVariant
ScrobblesModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if ( index.row() >= m_scrobbleList.size() )
        return QVariant();

    if ( index.column() >= m_headerTitles.size() )
        return QVariant();


    if ( role == Qt::DisplayRole )
        return m_scrobbleList.at( index.row() ).attribute( index.column() );
    else if ( role == Qt::TextColorRole && !m_readOnly )
    {
        if ( !lastfm::ScrobbleCache::isValid( m_scrobbleList.at( index.row() ).track() ) )
            return QColor( Qt::red );
    }
    else if ( role == Qt::ToolTipRole && !m_readOnly )
    {
        Scrobble s = m_scrobbleList.at( index.row() );
        lastfm::ScrobbleCache::Invalidity invalidity;

        if ( !lastfm::ScrobbleCache::isValid( s.track(), &invalidity ) )
        {
            switch ( invalidity )
            {
            case lastfm::ScrobbleCache::TooShort:
                return tr( "This track is under 30 seconds" );
            case lastfm::ScrobbleCache::ArtistNameMissing:
                return tr( "The artist name is missing" );
            case lastfm::ScrobbleCache::TrackNameMissing:
                return tr( "Invalid track title" );
            case lastfm::ScrobbleCache::ArtistInvalid:
                return tr( "Invalid artist" );
            case lastfm::ScrobbleCache::NoTimestamp:
                return tr( "There is no timestamp" );
            case lastfm::ScrobbleCache::FromTheFuture:
                return tr( "This track is too far in the future" );
            case lastfm::ScrobbleCache::FromTheDistantPast:
                return tr( "This track was played over two weeks ago" );
            }
        }
    }
    else if ( role == Qt::CheckStateRole && index.column() == ScrobblesModel::Artist )
    {
        if ( m_readOnly )
            return QVariant();
        else if ( !lastfm::ScrobbleCache::isValid( m_scrobbleList.at( index.row() ).track() ) )
            return Qt::Unchecked;
        else
            return m_scrobbleList.at( index.row() ).isScrobblingEnabled() ? Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

QVariant
ScrobblesModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( role != Qt::DisplayRole )
        return QVariant();

    if ( orientation == Qt::Horizontal )
        return m_headerTitles.at( section );
    else
        return QVariant();
}

Qt::ItemFlags
ScrobblesModel::flags( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return Qt::ItemIsEnabled;

    if ( !m_readOnly )
    {
        if ( index.column() == ScrobblesModel::Artist )
            return QAbstractItemModel::flags( index ) | Qt::ItemIsUserCheckable;
        if ( index.column() == ScrobblesModel::Plays )
            return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
    }

    return QAbstractItemModel::flags( index );
}

bool
ScrobblesModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if ( index.isValid() )
    {
        Scrobble s = m_scrobbleList.at( index.row() );

        if ( index.column() == ScrobblesModel::Artist && role == Qt::CheckStateRole )
        {
            s.setEnableScrobbling( value.toBool() );
            m_scrobbleList.replace( index.row(), s );
            emit dataChanged( index, index );
            return true;
        }
        else if ( index.column() == ScrobblesModel::Plays )
        {
            bool ok;
            int count = value.toInt( &ok );

            if ( ok && count != 0 && count <= s.originalPlayCount() )
            {
                lastfm::MutableTrack( s.track() ).setExtra( "playCount", QString::number( count ) );
                emit dataChanged( index, index );
            }

            return true;
        }
    }

    return false;
}

QList<lastfm::Track>
ScrobblesModel::tracksToScrobble() const
{
    QList<lastfm::Track> tracks;

    for ( int i = 0 ; i < m_scrobbleList.count() ; i ++ )
        if ( m_scrobbleList.at( i ).isScrobblingEnabled() )
            tracks.append( m_scrobbleList.at( i ).track() );

    return tracks;
}
