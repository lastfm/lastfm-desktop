

#include "lib/unicorn/dialogs/ScrobbleConfirmationDialog.h"
#include "lib/unicorn/UnicornApplication.h"
#include "lib/unicorn/QMessageBoxBuilder.h"

#include "../Application.h"
#include "../Dialogs/CloseAppsDialog.h"
#include "IpodDevice.h"
#include "DeviceScrobbler.h"
#include "../Services/ScrobbleService/ScrobbleService.h"

#ifdef Q_WS_X11
#include <QFileDialog>
#endif
#include <QDebug>
#include <QDirIterator>
#include <QTimer>

#ifdef Q_OS_MAC
// Check for iTunes playcount difference once every 3 minutes
// (usually takes about 1 sec on Mac)
#define BACKGROUND_CHECK_INTERVAL 3 * 60 * 1000
#else
// On Windows the iPod scrobble check can take around 90 seconds
// for a fairly large library, so only run it every 30 minutes
#define BACKGROUND_CHECK_INTERVAL 30 * 60 * 1000
#endif

QString getIpodMountPath();

DeviceScrobbler::DeviceScrobbler( QObject *parent )
    :QObject( parent )
{
    m_twiddlyTimer = new QTimer( this );
    connect( m_twiddlyTimer, SIGNAL(timeout()), SLOT(twiddle()) );
    m_twiddlyTimer->start( BACKGROUND_CHECK_INTERVAL );

    // run once 3 seconds after starting up
    QTimer::singleShot( 3 * 1000, this, SLOT(twiddle()) );
}

DeviceScrobbler::~DeviceScrobbler()
{
    if ( m_confirmDialog )
        m_confirmDialog->deleteLater();

    // make sure we terminate twiddly we could be clsoed due
    // to the installer and twiddly would stop install working
    if ( m_twiddly )
        m_twiddly->deleteLater();
}

bool
DeviceScrobbler::isITunesPluginInstalled()
{
#ifdef Q_OS_WIN
    QSettings settings( "HKEY_LOCAL_MACHINE\\SOFTWARE\\Last.fm\\Client\\Plugins\\itw", QSettings::NativeFormat );
    QFile pluginFile( settings.value( "Path" ).toString() );
    return pluginFile.exists();
#else
    return true;
#endif
}

void
DeviceScrobbler::twiddle()
{
#ifndef Q_WS_X11
    if ( CloseAppsDialog::isITunesRunning() && isITunesPluginInstalled() )
    {
        if (m_twiddly)
        {
            qWarning() << "m_twiddly already running. Early out.";
            return;
        }

        //"--device diagnostic --vid 0000 --pid 0000 --serial UNKNOWN

        QStringList args = (QStringList()
                            << "--device" << "background"
                            << "--vid" << "0000"
                            << "--pid" << "0000"
                            << "--serial" << "UNKNOWN");

        if ( false )
            args += "--manual";

        m_twiddly = new QProcess( this );
        connect( m_twiddly, SIGNAL(finished( int, QProcess::ExitStatus )), SLOT(onTwiddlyFinished( int, QProcess::ExitStatus )) );
        connect( m_twiddly, SIGNAL(error( QProcess::ProcessError )), SLOT(onTwiddlyError( QProcess::ProcessError )) );
#ifdef Q_OS_WIN
        m_twiddly->start( QDir( QCoreApplication::applicationDirPath() ).absoluteFilePath( "iPodScrobbler.exe" ), args );
#else
        m_twiddly->start( QDir( QCoreApplication::applicationDirPath() ).absoluteFilePath( "../Helpers/iPodScrobbler" ), args );
#endif
    }
#endif //  Q_WS_X11
}

void
DeviceScrobbler::onTwiddlyFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    qDebug() << exitCode << exitStatus;
    m_twiddly->deleteLater();
}

void
DeviceScrobbler::onTwiddlyError( QProcess::ProcessError error )
{
    qDebug() << error;
    m_twiddly->deleteLater();
}

void 
DeviceScrobbler::checkCachedIPodScrobbles()
{
    QStringList files;

    // check if there are any iPod scrobbles in its folder
    QDir scrobblesDir = lastfm::dir::runtimeData();

    if ( scrobblesDir.cd( "devices" ) )
    {
        QDirIterator iterator( scrobblesDir, QDirIterator::Subdirectories );

        while ( iterator.hasNext() )
        {
            iterator.next();

            if ( iterator.fileInfo().isFile() )
            {
                QString filename = iterator.fileName();

                if ( filename.endsWith(".xml") )
                    files << iterator.fileInfo().absoluteFilePath();
            }
        }
    }

    scrobbleIpodFiles( files );
}



void 
DeviceScrobbler::handleMessage( const QStringList& message )
{
    int pos = message.indexOf( "--twiddly" );
    const QString& action = message[ pos + 1 ];
    
    if( action == "complete" )
        twiddled( message );
}


void 
DeviceScrobbler::iPodDetected( const QStringList& /*arguments*/ )
{
}

void 
DeviceScrobbler::twiddled( const QStringList& arguments )
{
    // iPod scrobble time!
    QString iPodPath = arguments[ arguments.indexOf( "--ipod-path" ) + 1 ];

    if ( !arguments.contains( "no-tracks-found" ) )
        scrobbleIpodFiles( QStringList( iPodPath ) );
}



void 
DeviceScrobbler::scrobbleIpodFiles( const QStringList& files )
{
    qDebug() << files;

    bool removeFiles = false;

    if ( unicorn::AppSettings( OLDE_PLUGIN_SETTINGS ).value( SETTING_OLDE_ITUNES_DEVICE_SCROBBLING_ENABLED, true ).toBool() )
    {
        QList<lastfm::Track> scrobbles = scrobblesFromFiles( files );

        // TODO: fix the root cause of this problem
        // If there are more than 4000 scrobbles we assume there was an error with the
        // iPod scrobbling diff checker so discard these scrobbles.
        // 4000 because 16 waking hours a day, for two weeks, with 3.5 minute songs
        if ( scrobbles.count() >= 4000 )
            removeFiles = true;
        else
        {
            if ( scrobbles.count() > 0 )
            {
                if ( unicorn::AppSettings().value( SETTING_ALWAYS_ASK, true ).toBool()
                     || scrobbles.count() >= 200 ) // always get them to check scrobbles over 200
                {
                    if ( !m_confirmDialog )
                    {
                        m_confirmDialog = new ScrobbleConfirmationDialog( scrobbles, aApp->mainWindow() );
                        connect( m_confirmDialog, SIGNAL(finished(int)), SLOT(onScrobblesConfirmationFinished(int)) );
                    }
                    else
                        m_confirmDialog->addTracks( scrobbles );

                    // add the files so it can delete them when the user has decided what to do
                    m_confirmDialog->addFiles( files );
                    m_confirmDialog->show();
                }
                else
                {
                    // sort the iPod scrobbles before caching them
                    if ( scrobbles.count() > 1 )
                        qSort ( scrobbles.begin(), scrobbles.end() );

                    emit foundScrobbles( scrobbles );

                    // we're scrobbling them so remove the source files
                    removeFiles = true;
                }
            }
            else
                // there were no scrobbles in the files so remove them
                removeFiles = true;
        }
    }
    else
        // device scrobbling is disabled so remove these files
        removeFiles = true;

    if ( removeFiles )
        foreach ( QString file, files )
            QFile::remove( file );

}

QList<lastfm::Track>
DeviceScrobbler::scrobblesFromFiles( const QStringList& files  )
{
    QList<lastfm::Track> scrobbles;

    foreach ( const QString file, files )
    {
        QFile iPodScrobbleFile( file );

        if ( iPodScrobbleFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            QDomDocument iPodScrobblesDoc;
            iPodScrobblesDoc.setContent( &iPodScrobbleFile );
            QDomNodeList tracks = iPodScrobblesDoc.elementsByTagName( "track" );

            for ( int i(0) ; i < tracks.count() ; ++i )
            {
                lastfm::Track track( tracks.at(i).toElement() );

                // don't add tracks to the list if they don't have an artist
                // don't add podcasts to the list if podcast scrobbling is off
                // don't add videos to the list (well, videos that aren't "music video")
                // don't add tracks if they are in excluded folders

                if ( !track.artist().isNull()
                     && ( unicorn::UserSettings().value( "podcasts", true ).toBool() || !track.isPodcast() )
                     && !track.isVideo()
                     && !ScrobbleService::isDirExcluded( track ) )
                    scrobbles << track;
            }
        }
    }

    return scrobbles;
}

void
DeviceScrobbler::onScrobblesConfirmationFinished( int result )
{
    if ( result == QDialog::Accepted )
    {
        QList<lastfm::Track> scrobbles = m_confirmDialog->tracksToScrobble();

        // sort the iPod scrobbles before caching them
        if ( scrobbles.count() > 1 )
            qSort ( scrobbles.begin(), scrobbles.end() );

        emit foundScrobbles( scrobbles );

        unicorn::AppSettings().setValue( SETTING_ALWAYS_ASK, !m_confirmDialog->autoScrobble() );
    }

    // delete all the iPod scrobble files whether it was accepted or not
    foreach ( const QString file, m_confirmDialog->files() )
        QFile::remove( file );

    m_confirmDialog->deleteLater();
}

#ifdef Q_WS_X11
void 
DeviceScrobbler::onScrobbleIpodTriggered() {
    if ( iPod )
    {
        qDebug() << "deleting ipod...";
        delete iPod;
    }
    qDebug() << "here";
    iPod = new IpodDeviceLinux;
    QString path;
    bool autodetectionSuceeded = true;

    if ( !iPod->autodetectMountPath() )
    {
        path = getIpodMountPath();
        iPod->setMountPath( path );
        autodetectionSuceeded = false;
    }

    if ( autodetectionSuceeded || !path.isEmpty() )
    {
        connect( iPod, SIGNAL( scrobblingCompleted( int ) ), this, SLOT( scrobbleIpodTracks( int ) ) );
        connect( iPod, SIGNAL( calculatingScrobbles( int ) ), this, SLOT( onCalculatingScrobbles( int ) ) );
        connect( iPod, SIGNAL( errorOccurred() ), this, SLOT( onIpodScrobblingError() ) );
        iPod->fetchTracksToScrobble();
    }
}


QString getIpodMountPath()
{
    QString path = "";
    QFileDialog dialog( 0, QObject::tr( "Where is your iPod mounted?" ), "/" );
    dialog.setOption( QFileDialog::ShowDirsOnly, true );
    dialog.setFileMode( QFileDialog::Directory );

    //The following lines are to make sure the QFileDialog looks native.
    QString backgroundColor( "transparent" );
    dialog.setStyleSheet( "QDockWidget QFrame{ background-color: " + backgroundColor + "; }" );

    if ( dialog.exec() )
    {
       path = dialog.selectedFiles()[ 0 ];
    }
    return path;
}

void 
DeviceScrobbler::onCalculatingScrobbles( int trackCount )
{
    qApp->setOverrideCursor( Qt::WaitCursor );
}

void 
DeviceScrobbler::scrobbleIpodTracks( int trackCount )
{
    qApp->restoreOverrideCursor();
    qDebug() << trackCount << " new tracks to scrobble.";

    bool bootStrapping = false;
    if ( iPod->lastError() != IpodDeviceLinux::NoError && !iPod->isDeviceKnown() )
    {
        bootStrapping = true;
        qDebug() << "Should we save it?";
        int result = QMessageBoxBuilder( 0 )
            .setIcon( QMessageBox::Question )
            .setTitle( tr( "Scrobble iPod" ) )
            .setText( tr( "Do you want to associate the device %1 to your audioscrobbler user account?" ).arg( iPod->deviceName() ) )
            .setButtons( QMessageBox::Yes | QMessageBox::No )
            .exec();

        if ( result == QMessageBox::Yes )
        {
            iPod->associateDevice();
            QMessageBoxBuilder( 0 )
                .setIcon( QMessageBox::Information )
                .setTitle( tr( "Scrobble iPod" ) )
                .setText( tr( "Device successfully associated to your user account. "
                            "From now on you can scrobble the tracks you listen on this device." ) )
                .exec();

        }
        else
        {
            IpodDeviceLinux::deleteDeviceHistory( qobject_cast<unicorn::Application*>( qApp )->currentSession().user().name(), iPod->deviceId() );
        }
    }

    QList<lastfm::Track> tracks = iPod->tracksToScrobble();

    if ( tracks.count() )
    {
        if ( !bootStrapping )
        {
            if( unicorn::UserSettings().value( "confirmIpodScrobbles", false ).toBool() )
            {
                qDebug() << "showing confirm dialog";
                ScrobbleConfirmationDialog confirmDialog( tracks );
                if ( confirmDialog.exec() == QDialog::Accepted )
                {
                    tracks = confirmDialog.tracksToScrobble();

                    // sort the iPod scrobbles before caching them
                    if ( tracks.count() > 1 )
                        qSort ( tracks.begin(), tracks.end() );

                    emit foundScrobbles( tracks );
                }
            }
            else
            {
                // sort the iPod scrobbles before caching them
                if ( tracks.count() > 1 )
                    qSort ( tracks.begin(), tracks.end() );

                emit foundScrobbles( tracks );
                QMessageBoxBuilder( 0 )
                    .setIcon( QMessageBox::Information )
                    .setTitle( tr( "Scrobble iPod" ) )
                    .setText( tr( "%1 tracks scrobbled." ).arg( tracks.count() ) )
                    .exec();
            }
        }
    }
    else if ( !iPod->lastError() )
    {
        QMessageBoxBuilder( 0 )
            .setIcon( QMessageBox::Information )
            .setTitle( tr( "Scrobble iPod" ) )
            .setText( tr( "No tracks to scrobble since your last sync." ) )
            .exec();
        qDebug() << "No tracks to scrobble";
    }
    delete iPod;
    iPod = 0;
}

void 
DeviceScrobbler::onIpodScrobblingError()
{
    qDebug() << "iPod Error";
    qApp->restoreOverrideCursor();
    QString path;
    switch( iPod->lastError() )
    {
        case IpodDeviceLinux::AutodetectionError: //give it another try
            qDebug() << "giving another try";
            path = getIpodMountPath();
            if ( !path.isEmpty() )
            {
                iPod->setMountPath( path );
                iPod->fetchTracksToScrobble();
            }
            break;

        case IpodDeviceLinux::AccessError:
            QMessageBoxBuilder( 0 )
                .setIcon( QMessageBox::Critical )
                .setTitle( tr( "Scrobble iPod" ) )
                .setText( tr( "The iPod database could not be opened." ) )
                .exec();
            delete iPod;
            iPod = 0;
            break;
        case IpodDeviceLinux::UnknownError:
            QMessageBoxBuilder( 0 )
                .setIcon( QMessageBox::Critical )
                .setTitle( tr( "Scrobble iPod" ) )
                .setText( tr( "An unknown error occurred while trying to access the iPod database." ) )
                .exec();
            delete iPod;
            iPod = 0;
            break;
        default:
            qDebug() << "untracked error:" << iPod->lastError();
    }
}

#endif


