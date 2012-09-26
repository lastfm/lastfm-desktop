
#include <QMovie>

#include <lastfm/Library.h>

#include <lib/unicorn/dialogs/ShareDialog.h>
#include <lib/unicorn/dialogs/TagDialog.h>
#include <lib/unicorn/DesktopServices.h>
#include <lib/unicorn/TrackImageFetcher.h>

#include "../Application.h"
#include "../Services/RadioService/RadioService.h"
#include "../Services/ScrobbleService/ScrobbleService.h"

#include "TrackWidget.h"
#include "ui_TrackWidget.h"

TrackWidget::TrackWidget( Track& track, QWidget *parent )
    :QPushButton(parent),
    ui( new Ui::TrackWidget ),
    m_nowPlaying( false ),
    m_triedFetchAlbumArt( false )
{
    ui->setupUi( this );

    m_spinner = new QLabel( this );
    m_spinner->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    m_spinner->hide();

    m_movie = new QMovie( ":/icon_eq.gif", "GIF", this );
    m_movie->setCacheMode( QMovie::CacheAll );
    ui->equaliser->setMovie( m_movie );

    ui->buttonLayout->setAlignment( ui->love, Qt::AlignTop );
    ui->buttonLayout->setAlignment( ui->tag, Qt::AlignTop );
    ui->buttonLayout->setAlignment( ui->share, Qt::AlignTop );
    ui->buttonLayout->setAlignment( ui->buy, Qt::AlignTop );
    ui->trackTitleLayout->setAlignment( ui->asterisk, Qt::AlignTop );

    ui->albumArt->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->love->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->tag->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->share->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->buy->setAttribute( Qt::WA_LayoutUsesWidgetRect );

    setAttribute( Qt::WA_MacNoClickThrough );
    ui->albumArt->setAttribute( Qt::WA_MacNoClickThrough );
    ui->love->setAttribute( Qt::WA_MacNoClickThrough );
    ui->tag->setAttribute( Qt::WA_MacNoClickThrough );
    ui->share->setAttribute( Qt::WA_MacNoClickThrough );
    ui->buy->setAttribute( Qt::WA_MacNoClickThrough );

    connect( ui->love, SIGNAL(clicked(bool)), SLOT(onLoveClicked(bool)));
    connect( ui->tag, SIGNAL(clicked()), SLOT(onTagClicked()));
    connect( ui->share, SIGNAL(clicked()), SLOT(onShareClicked()));
    connect( ui->buy, SIGNAL(clicked()), SLOT(onBuyClicked()));

    connect( this, SIGNAL(clicked()), SLOT(onClicked()) );

    setTrack( track );
}

TrackWidget::~TrackWidget()
{
    delete ui;
}

QSize
TrackWidget::sizeHint() const
{
    QSize sizeHint = ui->frame->sizeHint();
    sizeHint.setWidth( QPushButton::sizeHint().width() );
    return sizeHint;
}

void
TrackWidget::onClicked()
{
    emit clicked( *this );
}

void
TrackWidget::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu* contextMenu = new QMenu( this );

    if ( ! m_nowPlaying )
    {
        contextMenu->addAction( tr( "Delete this scrobble from your profile" ), this, SLOT(onRemoveClicked()));
        contextMenu->addSeparator();
    }

    contextMenu->addAction( tr( "Play %1 Radio" ).arg( m_track.artist().name() ), this, SLOT(play()));

    if ( RadioService::instance().state() == Playing )
        contextMenu->addAction( tr( "Cue %1 Radio" ).arg( m_track.artist().name() ), this, SLOT(playNext()));

    if ( contextMenu->actions().count() )
        contextMenu->exec( event->globalPos() );
}

void
TrackWidget::play()
{
    lastfm::RadioStation rs = lastfm::RadioStation::similar( m_track.artist().name() );
    rs.setTitle( tr( "%1 Radio" ).arg( m_track.artist().name() ) );
    RadioService::instance().play( rs );
}

void
TrackWidget::playNext()
{
    lastfm::RadioStation rs = lastfm::RadioStation::similar( m_track.artist().name() );
    rs.setTitle( tr( "%1 Radio" ).arg( m_track.artist().name() ) );
    RadioService::instance().playNext( rs );
}

void
TrackWidget::startSpinner()
{
    m_spinner->setGeometry( rect() );

    if ( !m_spinnerMovie )
    {
        m_spinnerMovie = new QMovie( ":/loading_meta.gif", "GIF", this );
        m_spinnerMovie->setCacheMode( QMovie::CacheAll );
        m_spinner->setMovie( m_spinnerMovie );
    }

    setEnabled( false );

    m_spinnerMovie->start();
    m_spinner->show();
}

void
TrackWidget::clearSpinner()
{
    if ( !m_spinnerMovie )
    {
        m_spinnerMovie = new QMovie( ":/loading_meta.gif", "GIF", this );
        m_spinnerMovie->setCacheMode( QMovie::CacheAll );
        m_spinner->setMovie( m_spinnerMovie );
    }

    setEnabled( true );

    m_spinnerMovie->stop();
    m_spinner->hide();
}

void
TrackWidget::showEvent(QShowEvent *)
{
    if ( m_nowPlaying )
        m_movie->start();
    fetchAlbumArt();
}

void
TrackWidget::hideEvent( QHideEvent * event )
{
    m_movie->stop();
}

void
TrackWidget::fetchAlbumArt()
{
    if ( isVisible() && !m_triedFetchAlbumArt )
    {
        m_triedFetchAlbumArt = true;

        delete m_trackImageFetcher;
        m_trackImageFetcher = new TrackImageFetcher( m_track, Track::MediumImage );
        connect( m_trackImageFetcher, SIGNAL(finished(QPixmap)), ui->albumArt, SLOT(setPixmap(QPixmap)) );
        m_trackImageFetcher->startAlbum();
    }
}

void
TrackWidget::resizeEvent(QResizeEvent *)
{
    setTrackTitleWidth();
}

void
TrackWidget::setTrackTitleWidth()
{
    int width = qMin( ui->trackTitleFrame->width(), ui->trackTitle->fontMetrics().width( ui->trackTitle->text() ) + 1 );

    if ( ui->asterisk->isVisible() )
        width = qMin( width, ui->trackTitleFrame->width() - (ui->asterisk->width() + 3) );

    ui->trackTitle->setFixedWidth( width );
}

void
TrackWidget::setTrack( lastfm::Track& track )
{
    disconnect( m_track.signalProxy(), 0, this, 0 );

    m_track = track;

    connect( m_track.signalProxy(), SIGNAL(loveToggled(bool)), SLOT(onLoveToggled(bool)) );
    connect( m_track.signalProxy(), SIGNAL(scrobbleStatusChanged(short)), SLOT(onScrobbleStatusChanged()));
    connect( m_track.signalProxy(), SIGNAL(corrected(QString)), SLOT(onCorrected(QString)));

    m_movie->stop();
    ui->equaliser->hide();
    ui->asterisk->hide();

    setTrackDetails();

    ui->albumArt->setPixmap( QPixmap( ":/meta_album_no_art.png" ) );
    ui->albumArt->setHref( track.www() );

    m_triedFetchAlbumArt = false;
    fetchAlbumArt();
}

void
TrackWidget::setTrackDetails()
{
    ui->trackTitle->setText( m_track.title( lastfm::Track::Corrected ) );
    ui->artist->setText( m_track.artist( lastfm::Track::Corrected ).name() );

    if ( m_track.title( lastfm::Track::Corrected ) != m_track.title( lastfm::Track::Original )
         || m_track.title( lastfm::Track::Corrected ) != m_track.title( lastfm::Track::Original ) )
    {
         ui->asterisk->show();
         ui->asterisk->setToolTip( tr( "Auto-corrected from: %1" ).arg( m_track.toString( lastfm::Track::Original ) ) );
    }

    if ( m_timestampTimer ) m_timestampTimer->stop();

    if ( m_track.scrobbleStatus() == lastfm::Track::Cached )
        ui->timestamp->setText( tr( "Cached" ) );
    else if ( m_track.scrobbleStatus() == lastfm::Track::Error )
        ui->timestamp->setText( tr( "Error: %1" ).arg( m_track.scrobbleErrorText() ) );
    else
        updateTimestamp();

    ui->love->setChecked( m_track.isLoved() );

    setTrackTitleWidth();
}

void
TrackWidget::onLoveToggled( bool loved )
{
    ui->love->setChecked( loved );
}

void
TrackWidget::onScrobbleStatusChanged()
{
    setTrackDetails();
}

void
TrackWidget::onCorrected( QString /*correction*/ )
{
    setTrackDetails();
}



lastfm::Track
TrackWidget::track() const
{
    return m_track;
}

void
TrackWidget::onLoveClicked( bool loved )
{
    if ( loved )
        MutableTrack( m_track ).love();
    else
        MutableTrack( m_track ).unlove();
}

void
TrackWidget::onTagClicked()
{
    TagDialog* td = new TagDialog( m_track, window() );
    td->raise();
    td->show();
    td->activateWindow();
}

void
TrackWidget::onShareClicked()
{
    QMenu* shareMenu = new QMenu( this );

    shareMenu->addAction( tr( "Share on Last.fm" ), this, SLOT(onShareLastFm()) );
    shareMenu->addAction( tr( "Share on Twitter" ), this, SLOT(onShareTwitter()) );
    shareMenu->addAction( tr( "Share on Facebook" ), this, SLOT(onShareFacebook()) );

    shareMenu->exec( cursor().pos() );
}

void
TrackWidget::onBuyClicked()
{
    if ( !ui->buy->menu() )
    {
        // show the buy links please!
        QString country = aApp->currentSession()->userInfo().country();
        qDebug() << country;

        if ( country.compare( "us", Qt::CaseInsensitive ) == 0 )
            country = "united states";
        else if ( country.compare( "de", Qt::CaseInsensitive ) == 0 )
            country = "germany";
        else
            country = "united kingdom";

        connect( m_track.getBuyLinks( country ), SIGNAL(finished()), SLOT(onGotBuyLinks()));
    }
}

QString
TrackWidget::price( const QString& price, const QString& currency ) const
{
    QString returnPrice;

    if ( currency.compare( "eur", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "€%1" ).arg( price );
    else if ( currency.compare( "usd", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "$%1" ).arg( price );
    else if ( currency.compare( "gbp", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "£%1" ).arg( price );
    else
        returnPrice = QString( "%1 %2" ).arg( price, currency );

    return returnPrice;
}

void
TrackWidget::onRemoveClicked()
{
    connect( lastfm::Library::removeScrobble( m_track ), SIGNAL(finished()), SLOT(onRemovedScrobble()));
}

void
TrackWidget::onRemovedScrobble()
{
   lastfm::XmlQuery lfm;

   if ( lfm.parse( static_cast<QNetworkReply*>( sender() )->readAll() ) )
   {
       qDebug() << lfm;

       emit removed();
   }
   else
   {
       qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
   }


}

void
TrackWidget::setNowPlaying( bool nowPlaying )
{
    m_nowPlaying = nowPlaying;
    updateTimestamp();
}

void
TrackWidget::updateTimestamp()
{
    if ( !m_timestampTimer )
    {
        m_timestampTimer = new QTimer( this );
        m_timestampTimer->setSingleShot( true );
        connect( m_timestampTimer, SIGNAL(timeout()), SLOT(updateTimestamp()));
    }

    if ( m_nowPlaying )
    {
        if ( isVisible() )
            m_movie->start();
        ui->equaliser->show();

        ui->timestamp->setText( tr( "Now listening" ) );
        ui->timestamp->setToolTip( "" );
    }
    else
    {        
        m_movie->stop();
        ui->equaliser->hide();

        unicorn::Label::prettyTime( *ui->timestamp, m_track.timestamp(), m_timestampTimer );
    }
}

void
TrackWidget::onGotBuyLinks()
{
    if ( !ui->buy->menu() )
    {
        // make sure we don't process this again if the button was clicked twice

        XmlQuery lfm;

        if ( lfm.parse( qobject_cast<QNetworkReply*>(sender())->readAll() ) )
        {
            bool thingsToBuy = false;

            QMenu* menu = new QMenu( this );

            menu->addAction( tr("Downloads") )->setEnabled( false );

            // USD EUR GBP

            foreach ( const XmlQuery& affiliation, lfm["affiliations"]["downloads"].children( "affiliation" ) )
            {
                bool isSearch = affiliation["isSearch"].text() == "1";

                QAction* buyAction = 0;

                if ( isSearch )
                    buyAction = menu->addAction( tr("Search on %1").arg( affiliation["supplierName"].text() ) );
                else
                    buyAction = menu->addAction( tr("Buy on %1 %2").arg( affiliation["supplierName"].text(), price( affiliation["price"]["amount"].text(), affiliation["price"]["currency"].text() ) ) );

                buyAction->setData( affiliation["buyLink"].text() );

                thingsToBuy = true;
            }

            menu->addSeparator();
            menu->addAction( tr("Physical") )->setEnabled( false );

            foreach ( const XmlQuery& affiliation, lfm["affiliations"]["physicals"].children( "affiliation" ) )
            {
                bool isSearch = affiliation["isSearch"].text() == "1";

                QAction* buyAction = 0;

                if ( isSearch )
                    buyAction = menu->addAction( tr("Search on %1").arg( affiliation["supplierName"].text() ) );
                else
                    buyAction = menu->addAction( tr("Buy on %1 %2").arg( affiliation["supplierName"].text(), price( affiliation["price"]["amount"].text(), affiliation["price"]["currency"].text() ) ) );

                buyAction->setData( affiliation["buyLink"].text() );

                thingsToBuy = true;
            }

            ui->buy->setMenu( menu );
            connect( menu, SIGNAL(triggered(QAction*)), SLOT(onBuyActionTriggered(QAction*)) );

            ui->buy->click();
        }
        else
        {
            // TODO: what happens when we fail?
            qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
        }
    }
}

void
TrackWidget::onBuyActionTriggered( QAction* buyAction )
{
    unicorn::DesktopServices::openUrl( buyAction->data().toString() );
}

void
TrackWidget::onShareLastFm()
{
    ShareDialog* sd = new ShareDialog( m_track, window() );
    sd->raise();
    sd->show();
    sd->activateWindow();
}

void
TrackWidget::onShareTwitter()
{
    ShareDialog::shareTwitter( m_track );
}

void
TrackWidget::onShareFacebook()
{
    ShareDialog::shareFacebook( m_track );
}

