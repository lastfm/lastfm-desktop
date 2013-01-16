/*
   Copyright 2005-2010 Last.fm Ltd.
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

#include "Application.h"
#include "IpodDevice_linux.h"
#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/UnicornSession.h"

#include <QApplication>
#include <QByteArray>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

extern "C"
{
    #include <gpod/itdb.h>
    #include <glib.h>
}

IpodTracksFetcher::IpodTracksFetcher( Itdb_iTunesDB *itdb, QSqlDatabase scrobblesdb,
                                      const QString& tableName, const QString& ipodModel )
{
    m_itdb = itdb;
    m_tableName = tableName;
    m_scrobblesdb = scrobblesdb;
    m_ipodModel = ipodModel;
}

void
IpodTracksFetcher::run()
{
    fetchTracks();
    exec();
}

void
IpodTracksFetcher::fetchTracks()
{
    GList *cur;
    for ( cur = m_itdb->tracks; cur; cur = cur->next )
    {
        Itdb_Track *iTrack = ( Itdb_Track * )cur->data;
        if ( !iTrack )
            continue;

        int newPlayCount = iTrack->playcount - previousPlayCount( iTrack );
        QDateTime time;
        time.setTime_t( iTrack->time_played );

        if ( time.toTime_t() == 0 )
            continue;

        QDateTime prevPlayTime = previousPlayTime( iTrack );

        //this logic takes into account that sometimes the itdb track play count is not
        //updated correctly (or libgpod doesn't get it right),
        //so we rely on the track play time too, which seems to be right most of the time
        if ( ( iTrack->playcount > 0 && newPlayCount > 0 ) || time > prevPlayTime )
        {
            Track lstTrack;
            setTrackInfo( lstTrack, iTrack );

            if ( newPlayCount == 0 )
                newPlayCount++;

            //add the track to the list as many times as the updated playcount.
            for ( int i = 0; i < newPlayCount; i++ )
            {
                m_tracksToScrobble.append( lstTrack );
            }

            commit( iTrack );
        }
    }
    qDebug() << "tracks fetching finished";
    exit();
}

void
IpodTracksFetcher::setTrackInfo( Track& lstTrack, Itdb_Track* iTrack )
{
    MutableTrack( lstTrack ).setArtist( QString::fromUtf8( iTrack->artist ) );
    MutableTrack( lstTrack ).setAlbum( QString::fromUtf8( iTrack->album ) );
    MutableTrack( lstTrack ).setTitle( QString::fromUtf8( iTrack->title ) );
    MutableTrack( lstTrack ).setSource( Track::MediaDevice );
    if ( iTrack->mediatype & ITDB_MEDIATYPE_PODCAST )
        MutableTrack( lstTrack ).setPodcast( true );

    QDateTime t;
    t.setTime_t( iTrack->time_played );
    MutableTrack( lstTrack ).setTimeStamp( t );
    MutableTrack( lstTrack ).setDuration( iTrack->tracklen / 1000 ); // set duration in seconds

    MutableTrack( lstTrack ).setExtra( "playerName", "iPod " + m_ipodModel );
}

void
IpodTracksFetcher::commit( Itdb_Track* iTrack )
{
    QSqlQuery query( m_scrobblesdb );
    QString sql = "REPLACE INTO " + m_tableName + " ( playcount, lastplaytime, id ) VALUES( %1, %2, %3 )";

    query.exec( sql.arg( iTrack->playcount ).arg( iTrack->time_played ).arg( iTrack->id  ) );
    if( query.lastError().type() != QSqlError::NoError )
        qWarning() << query.lastError().text();
}

uint
IpodTracksFetcher::previousPlayCount( Itdb_Track* track ) const
{
    QSqlQuery query( m_scrobblesdb );
    QString sql = "SELECT playcount FROM " + m_tableName + " WHERE id=" + QString::number( track->id );

    query.exec( sql );

    if( query.next() )
        return query.value( 0 ).toUInt();
    return 0;
}

QDateTime
IpodTracksFetcher::previousPlayTime( Itdb_Track* track ) const
{
    QSqlQuery query( m_scrobblesdb );
    QString sql = "SELECT lastplaytime FROM " + m_tableName  + " WHERE id=" + QString::number( track->id );

    query.exec( sql );

    if( query.next() )
        return QDateTime::fromTime_t( query.value( 0 ).toUInt() );
    return QDateTime::fromTime_t( 0 );
}

IpodDeviceLinux::IpodDeviceLinux()
    : m_itdb( 0 )
    , m_mpl( 0 )
    , m_tf( 0 )
    , m_autodetected( false )
    , m_error( NoError )
{}


IpodDeviceLinux::~IpodDeviceLinux()
{
    if ( m_itdb )
    {
        itdb_free( m_itdb );
        itdb_playlist_free( m_mpl );
    }

    delete m_tf;
}

bool
IpodDeviceLinux::deleteDeviceHistory( QString username, QString deviceId )
{
    QString const name = DB_NAME;
    QSqlDatabase db = QSqlDatabase::database( name );
    bool b = false;

    if ( !db.isValid() )
    {
        db = QSqlDatabase::addDatabase( "QSQLITE", name );
        db.setDatabaseName( lastfm::dir::runtimeData().filePath( name + ".db" ) );
    }

    db.open();

    QSqlQuery q( db );
    b = q.exec( "DROP TABLE " + username + "_" + deviceId );

    if ( !b )
        qWarning() << q.lastError().text();

    return b;
}

bool
IpodDeviceLinux::deleteDevicesHistory()
{
    QString const name = DB_NAME;
    QString filePath = lastfm::dir::runtimeData().filePath( name + ".db" );
    return QFile::remove( filePath );
}


QSqlDatabase
IpodDeviceLinux::database() const
{
    QString const name = DB_NAME;
    QSqlDatabase db = QSqlDatabase::database( name );

    if ( !db.isValid() )
    {
        db = QSqlDatabase::addDatabase( "QSQLITE", name );
        db.setDatabaseName( lastfm::dir::runtimeData().filePath( name + ".db" ) );

        db.open();


    }

    if( !db.tables().contains( tableName() ) )
    {
        QSqlQuery q( db );
        qDebug() << "table name: " << tableName();
        bool b = q.exec( "CREATE TABLE " + tableName() + " ( "
                         "id           INTEGER PRIMARY KEY, "
                         "playcount    INTEGER, "
                         "lastplaytime INTEGER )" );
        if ( !b )
            qWarning() << q.lastError().text();
    }

    return db;
}

void
IpodDeviceLinux::open()
{
    QByteArray _mountpath = QFile::encodeName( mountPath() );
    const char* mountpath = _mountpath.data();

    qDebug() << "mount path: " << mountPath();

    m_itdb = itdb_new();
    itdb_set_mountpoint( m_itdb, mountpath );
    m_mpl = itdb_playlist_new( "iPod", false );
    itdb_playlist_set_mpl( m_mpl );
    GError* err = 0;
    m_itdb = itdb_parse( mountpath, &err );

    if ( err )
        throw tr( "The iPod database could not be opened." );

    if( m_deviceId.isEmpty() )
    {
        const Itdb_IpodInfo* iPodInfo = itdb_device_get_ipod_info( m_itdb->device );
        const gchar* ipodModel = itdb_info_get_ipod_model_name_string( iPodInfo->ipod_model );
        m_ipodModel = QString( ipodModel );
        m_deviceId = m_ipodModel.section( ' ', 0, 0 ) + "_" + QString::number( m_itdb->id );
    }

}

const QList<Track>&
IpodDeviceLinux::tracksToScrobble() const
{
    return m_tracksToScrobble;
}

void
IpodDeviceLinux::fetchTracksToScrobble()
{
    try
    {
        open();
    }
    catch ( QString &error )
    {
        qDebug() << "Error initializing the device:" << error;
        if ( m_autodetected )
        {
            m_error = AutodetectionError;
        }
        else
        {
            m_error = AccessError;
        }
        emit errorOccurred();
        return;
    }
    
    emit calculatingScrobbles( itdb_tracks_number( m_itdb ) );
    m_tf = new IpodTracksFetcher( m_itdb, database(), tableName(), m_ipodModel );
    connect( m_tf, SIGNAL( finished() ), this, SLOT( onFinished() ) );
    m_tf->start();

}


void
IpodDeviceLinux::onFinished()
{
    m_error = NoError;
    m_tracksToScrobble = m_tf->tracksToScrobble();
    emit scrobblingCompleted( m_tracksToScrobble.count() );
}

QString
IpodDeviceLinux::deviceName() const
{
    QStringList devPath = mountPath().split( "/", QString::SkipEmptyParts );
    if ( !devPath.isEmpty() )
        return devPath.last();
    return QString();
}

QString
IpodDeviceLinux::tableName() const
{
    audioscrobbler::Application* app = qobject_cast<audioscrobbler::Application* >( qApp );
    if ( app )
    {
        return app->currentSession().user().name() + "_" + m_deviceId;
    }
    return QString();
}

bool
IpodDeviceLinux::autodetectMountPath()
{
    unicorn::UserSettings us;
    int count = us.beginReadArray( "associatedDevices" );

    if ( !count )
        return false;

    m_detectedDevices.clear();

    DeviceInfo deviceInfo;
    QString deviceId;
    for ( int i = 0; i < count; i++ )
    {
        us.setArrayIndex( i );
        deviceId = us.value( "deviceId" ).toString();
        deviceInfo.prettyName = us.value( "deviceName" ).toString();
        deviceInfo.mountPath = us.value( "mountPath" ).toString();
        if ( QFile::exists( deviceInfo.mountPath ) )
        {
            m_detectedDevices[ deviceId ] = deviceInfo;
        }
    }
    us.endArray();

    //No device detected or many, so user has to choose.
    if ( m_detectedDevices.count() == 0 || m_detectedDevices.count() > 1 )
    {
        return false;
    }

    setMountPath( m_detectedDevices.values()[ 0 ].mountPath, true );
    return true;
}

void
IpodDeviceLinux::setMountPath( const QString &path, bool autodetected )
{
    m_mountPath = path;
    m_autodetected = autodetected;
}
