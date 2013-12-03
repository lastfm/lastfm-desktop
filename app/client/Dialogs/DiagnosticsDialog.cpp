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

#include "lib/unicorn/UnicornCoreApplication.h"

#include "ui_DiagnosticsDialog.h"
#include "DiagnosticsDialog.h"
#include "../Services/ScrobbleService/ScrobbleService.h"
#include "../MediaDevices/DeviceScrobbler.h"

#include "common/c++/Logger.h"

#include <lastfm/Audioscrobbler.h>
#include <lastfm/misc.h>
#include <lastfm/ScrobbleCache.h>
#include <lastfm/ws.h>

#include <QByteArray>
#include <QDebug>
#include <QHeaderView>
#include <QProcess>

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

#ifndef Q_WS_X11
    QString path = unicorn::CoreApplication::log( "iPodScrobbler" ).absoluteFilePath();

    // we seek to the end below, but then twiddly's logger pretruncates the file
    // which then means our seeked position is beyond the file's end, and we
    // thus don't show any log output
#ifdef WIN32
    Logger::truncate( (wchar_t*) path.utf16() );
#else
    QByteArray const cpath = QFile::encodeName( path );
    Logger::truncate( cpath.data() );
#endif

    m_ipod_log = new QFile( path, this );
    m_ipod_log->open( QIODevice::ReadOnly );
    m_ipod_log->seek( m_ipod_log->size() );
    ui->ipod_log->clear();

    QTimer* timer = new QTimer( this );
    timer->setInterval( 10 );
    connect( timer, SIGNAL(timeout()), SLOT(poll()) );
    timer->start();
#endif
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
        case Audioscrobbler::ErrorBadSession: return tr( "Your session expired, it is being renewed." );
        case Audioscrobbler::ErrorBannedClientVersion: return tr( "Your client too old, you must upgrade." );
        case Audioscrobbler::ErrorInvalidSessionKey: return tr( "Your username or password is incorrect" );
        case Audioscrobbler::ErrorBadTime: return tr( "Your timezone or date are incorrect" );
        case Audioscrobbler::ErrorThreeHardFailures: return tr( "The submissions server is down" );

        case Audioscrobbler::Connecting: return tr( "Connecting to Last.fm..." );
        case Audioscrobbler::Scrobbling: return tr( "Scrobbling..." );

        case Audioscrobbler::TracksScrobbled:
        case Audioscrobbler::Handshaken:
            return tr( "Ready" );
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
        ui->subs_cache_count->setText( tr("%n locally cached track(s)", "", items.count() ) );
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
    bool const isManual = ( ui->ipod_type->currentIndex() == 1 );
    DeviceScrobbler::DoTwiddlyResult doTwiddlyResult = ScrobbleService::instance().deviceScrobbler()->doTwiddle( isManual );

    switch ( doTwiddlyResult )
    {
    case DeviceScrobbler::AlreadyRunning:
        ui->ipod_log->appendPlainText( "ALREADY SCROBBLING IPOD..." );
        break;
    case DeviceScrobbler::ITunesNotRunning:
        ui->ipod_log->appendPlainText( "ITUNES NOT RUNNING!" );
        break;
    case DeviceScrobbler::ITunesPluginNotInstalled:
        ui->ipod_log->appendPlainText( "ITUNES PLUGIN NOT INSTALLED!" );
        break;
    default:
        // don't say anything; it was DoTwiddlyResult::Started
        break;
    }
}


void 
DiagnosticsDialog::onSendLogsClicked()
{
//TODO    SendLogsDialog( this ).exec();
}
