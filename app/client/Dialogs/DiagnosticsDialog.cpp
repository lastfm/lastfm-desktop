/*
   Copyright 2005-2009 Last.fm Ltd. 
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

#include <QByteArray>
#include <QDebug>
#include <QHeaderView>
#include <QProcess>

#include <lastfm/Audioscrobbler.h>
#include <lastfm/misc.h>
#include <lastfm/ScrobbleCache.h>
#include <lastfm/ws.h>

#include "lib/unicorn/UnicornCoreApplication.h"

#include "ui_DiagnosticsDialog.h"
#include "DiagnosticsDialog.h"

#include "common/c++/Logger.h"

DiagnosticsDialog::DiagnosticsDialog( QWidget *parent )
        : QDialog( parent ),
          ui( new Ui::DiagnosticsDialog ),
          m_ipod_log( 0 )
{    
    ui->setupUi( this );

    setAttribute( Qt::WA_DeleteOnClose );

    ui->cached->header()->setResizeMode( QHeaderView::Stretch );
    ui->fingerprints->header()->setResizeMode( QHeaderView::Stretch );
    
    m_delay = new DelayedLabelText( ui->subs_status );

    ui->tabs->removeTab( 0 );
    ui->tabs->removeTab( 0 );

#ifdef Q_WS_X11
    ui->tabs->removeTab( 0 );
#endif

#ifdef Q_OS_MAC
    ui->subs_status->setAttribute( Qt::WA_MacSmallSize );
    ui->subs_cache_count->setAttribute( Qt::WA_MacSmallSize );
    ui->fingerprints_title->setAttribute( Qt::WA_MacSmallSize );
    ui->cached->setAttribute( Qt::WA_MacSmallSize );
    ui->cached->setAttribute( Qt::WA_MacShowFocusRect, false );
    ui->fingerprints->setAttribute( Qt::WA_MacSmallSize );
    ui->fingerprints->setAttribute( Qt::WA_MacShowFocusRect, false );
    
    QFont f = ui->ipod_log->font();
    f.setPixelSize( 10 );
    ui->ipod_log->setFont( f );
#endif
    
    connect( qApp, SIGNAL(scrobblePointReached( Track )), SLOT(onScrobblePointReached()), Qt::QueuedConnection ); // queued because otherwise cache isn't filled yet
    connect( ui->ipod_scrobble_button, SIGNAL(clicked()), SLOT(onScrobbleIPodClicked()) );
    connect( ui->logs_button, SIGNAL(clicked()), SLOT(onSendLogsClicked()) );

    onScrobblePointReached();
}

DiagnosticsDialog::~DiagnosticsDialog()
{
    delete ui;
}
    /*
static QString scrobblerStatusText( int const i )
{
    using lastfm::Audioscrobbler;

    #define tr QObject::tr
    switch (i)
    {
        case Audioscrobbler::ErrorBadSession: return tr( "Su sesión terminó, está siendo renovada." );
        case Audioscrobbler::ErrorBannedClientVersion: return tr( "Su versión es demasiado antigua, debe actualizarla." );
        case Audioscrobbler::ErrorInvalidSessionKey: return tr( "Su nombre de usuario o contraseña es incorrecta" );
        case Audioscrobbler::ErrorBadTime: return tr( "Su zona horaria o fecha es incorrecta" );
        case Audioscrobbler::ErrorThreeHardFailures: return tr( "El servidor de sumisión está caído" );

        case Audioscrobbler::Connecting: return tr( "Conectándose a Last.fm..." );
        case Audioscrobbler::Scrobbling: return tr( "Scrobbling..." );

        case Audioscrobbler::TracksScrobbled:
        case Audioscrobbler::Handshaken:
            return tr( "Listo" );
    }
    #undef tr

    return "";
}
*/

void
DiagnosticsDialog::scrobbleActivity( int /*msg*/ )
{
    /*
    m_delay->add( scrobblerStatusText( msg ) );

    if (msg == Audioscrobbler::TracksScrobbled)
        QTimer::singleShot( 1000, this, SLOT(onScrobblePointReached()) );

    switch (msg)
    {
        case Audioscrobbler::ErrorBadSession:
            //TODO flashing
        case Audioscrobbler::Connecting:
            //NOTE we only get this on startup
            ui->subs_light->setColor( Qt::yellow );
            break;
        case Audioscrobbler::Handshaken:
        case Audioscrobbler::Scrobbling:
        case Audioscrobbler::TracksScrobbled:
            ui->subs_light->setColor( Qt::green );
            break;
        
        case Audioscrobbler::ErrorBannedClientVersion:
        case Audioscrobbler::ErrorInvalidSessionKey:
        case Audioscrobbler::ErrorBadTime:
        case Audioscrobbler::ErrorThreeHardFailures:
            ui->subs_light->setColor( Qt::red );
            break;
    }
    */
}


void
DiagnosticsDialog::onScrobblePointReached()
{    
    ScrobbleCache cache( lastfm::ws::Username );

    QList<QTreeWidgetItem *> items;
    foreach (Track t, cache.tracks())
        items.append( new QTreeWidgetItem( QStringList() << t.artist() << t.title() << t.album() ) );
    ui->cached->clear();
    ui->cached->insertTopLevelItems( 0, items );

    if (items.count())
        ui->subs_cache_count->setText( tr("%n tema(s) almacenadas localmente", "", items.count() ) );
    else
        ui->subs_cache_count->clear();
}

void
DiagnosticsDialog::fingerprinted( const Track& t )
{
    new QTreeWidgetItem( ui->fingerprints,
                         QStringList() << t.artist() << t.title() << t.album() );
}


void 
DiagnosticsDialog::poll()
{    
    QTextStream s( m_ipod_log );
    while (!s.atEnd())
        ui->ipod_log->appendPlainText( s.readLine() );
}

void
DiagnosticsDialog::onScrobbleIPodClicked()
{
#ifndef Q_WS_X11
    if (m_twiddly)
    {
        qWarning() << "m_twiddly actualmente en uso. Quitarlo.";
        return;
    }

    //"--device diagnostic --vid 0000 --pid 0000 --serial UNKNOWN
    
    QStringList args = (QStringList()
                    << "--device" << "diagnóstico"
                    << "--vid" << "0000"
                    << "--pid" << "0000"
                    << "--serial" << "DESCONOCIDO");

    bool const isManual = ( ui->ipod_type->currentIndex() == 1 );
    if (isManual)
        args += "--manual";

    QString path = unicorn::CoreApplication::log( "iPodScrobbler" ).absoluteFilePath();
    //path = path.remove( ".debug" ); //because we run the release twiddly always

    // we seek to the end below, but then twiddly's logger pretruncates the file
    // which then means our seeked position is beyond the file's end, and we
    // thus don't show any log output
#ifdef WIN32
    Logger::truncate( (wchar_t*) path.utf16() );
#else
    QByteArray const cpath = QFile::encodeName( path );
    Logger::truncate( cpath.data() );
#endif

    m_ipod_log = new QFile( path );
    m_ipod_log->open( QIODevice::ReadOnly );
    m_ipod_log->seek( m_ipod_log->size() );
    ui->ipod_log->clear();
    
    m_twiddly = new QProcess( this );
    connect( m_twiddly, SIGNAL(finished( int, QProcess::ExitStatus )), SLOT(onTwiddlyFinished( int, QProcess::ExitStatus )) );
    connect( m_twiddly, SIGNAL(error( QProcess::ProcessError )), SLOT(onTwiddlyError( QProcess::ProcessError )) );
#ifdef Q_OS_WIN
    m_twiddly->start( QDir( QCoreApplication::applicationDirPath() ).absoluteFilePath( "iPodScrobbler.exe" ), args );
#else
    m_twiddly->start( QDir( QCoreApplication::applicationDirPath() ).absoluteFilePath( "../Helpers/iPodScrobbler" ), args );
#endif
    m_ipod_log->setParent( m_twiddly );
    
    QTimer* timer = new QTimer( m_twiddly );
    timer->setInterval( 10 );
    connect( timer, SIGNAL(timeout()), SLOT(poll()) );
    timer->start();
#endif
}


void
DiagnosticsDialog::onTwiddlyFinished( int code, QProcess::ExitStatus status )
{    
    qDebug() << "Exit code:" << code << lastfm::qMetaEnumString<QProcess>( status, "ExitStatus" );
    poll(); //get last bit
    m_twiddly->deleteLater();
}


void
DiagnosticsDialog::onTwiddlyError( QProcess::ProcessError e )
{
    qDebug() << "Twiddly error:" << lastfm::qMetaEnumString<QProcess>( e, "ProcessError" );
    m_twiddly->deleteLater();
}


void 
DiagnosticsDialog::onSendLogsClicked()
{
//TODO    SendLogsDialog( this ).exec();
}
