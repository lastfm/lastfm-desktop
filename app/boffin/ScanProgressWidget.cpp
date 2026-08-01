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
#include "ScanProgressWidget.h"
#include <lastfm/ws.h>
#include <types/Track.h>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>

/** all quite messy, apologies --mxcl */

#define MAX_IMAGE_COUNT 15


ScanProgressWidget::ScanProgressWidget()
{
    m_done = false;
    m_artist_count = 0;
    m_track_count = 0;

    (new QBasicTimer)->start( 20, this );
    
    setBackgroundRole( QPalette::Base );
    setAutoFillBackground( true );
    setAttribute( Qt::WA_MacShowFocusRect, false );
    setAttribute( Qt::WA_MacSmallSize );
}


void
ScanProgressWidget::onNewDirectory( const QString& directory )
{
    paths += directory;
    if (paths.size() > 60) paths.pop_front();
    update();
}


void
ScanProgressWidget::onNewTrack( const Track& t )
{
    int& i = count( t.artist() );
    i++;

    m_artist_count = track_counts.size();
    m_track_count++;

    updateStatusMessage();

    if (t.url().isValid()) {
        paths += t.url().path();
        // so this is a time saving way to keep the list the size of the screen
        // it goes over a bit almost certainly, but it's ok
        if (paths.size() > 60)
            paths.pop_front();
    }
    
    if (i == 1 && images.size() < MAX_IMAGE_COUNT) {
        QObject* o = new ImageFucker( t.artist() );
        connect( o, SIGNAL(fucked()), SLOT(onImageFucked()) );
        o->setParent( this );
    }
}

void
ScanProgressWidget::onFinished()
{
    m_done = true;
    updateStatusMessage();
    repaint();
}


void
ScanProgressWidget::paintEvent( QPaintEvent* )
{
    QPainter p( this );    
    int y = height() - 6;

    p.setPen( Qt::lightGray );
    foreach (QString const path, paths)
    {
        y -= 18;
        p.drawText( 6, y, path );
    }

    for (int i = 0; i < images.count(); ++i)
    {
        p.setRenderHint( QPainter::Antialiasing );
        p.setRenderHint( QPainter::SmoothPixmapTransform );

        int y = (images[i]->y * (height() - images[i]->pixmap.height())) / 1000;

        QPointF pt( width() - images[i]->x, y );

        p.setOpacity( images[i]->opacity );
        p.drawImage( pt, images[i]->pixmap );

        QRectF rect( pt.x(), y + (images[i]->pixmap.height() * 0.75) + 4, images[i]->pixmap.width(), height() );
        QString text = images[i]->artist;
        text = p.fontMetrics().elidedText( text, Qt::ElideRight, rect.width() );

        int const n = count( images[i]->artist );
        text += "\n" + (n == 1 ? tr("1 track") : tr( "%L1 tracks" ).arg( n ));

        p.setPen( Qt::black );
        p.drawText( rect, Qt::AlignTop | Qt::AlignHCenter, text );
    }
}


void
ScanProgressWidget::timerEvent( QTimerEvent* )
{
    QList<int> remove;
    
    for (int i = 0; i < images.count(); ++i)
    {   
        int steps = images[i]->steps;
        
        images[i]->opacity = float(steps > 100 ? 200 - steps : steps) / 100.0f;        

#ifdef Q_WS_WIN
        // 0.5 pixel movements chug on windows
        images[i]->x += 1;
#else
        // 0.5 pixel movements look great on mac
        images[i]->x += 0.5;
#endif

        images[i]->steps++;

        if (steps == 200) remove.prepend( i );
    }
    
    bool needUpdate = images.count();

    foreach (int i, remove)
        images.takeAt( i )->deleteLater();

    if (needUpdate)
        update();
}


void
ScanProgressWidget::onImageFucked()
{
    ImageFucker* fucker = (ImageFucker*)sender();
    fucker->x = rand() % width();
    images += fucker;
}

void
ScanProgressWidget::updateStatusMessage()
{
    QString text;

    if (m_artist_count != 0 && m_track_count != 0)
    {
        text = tr("Found %L1 artists and %L2 tracks").arg( m_artist_count ).arg( m_track_count );
        if (m_done) text.prepend( tr("Scanning complete. ") );
    }
    else
        text = tr("Scanning...");

    emit statusMessage(text);
    //p.drawText( 6, height() - 6, text );
}


void
ImageFucker::onArtistGotInfo()
{
    QUrl url = Artist::getInfo( (QNetworkReply*)sender() ).imageUrl();
    QNetworkReply* reply = lastfm::nam()->get( QNetworkRequest(url) );
    connect( reply, SIGNAL(finished()), SLOT(onImageDownloaded()) );
}


static inline QImage reflect0rize( const QImage& in )
{
    const int H = in.height() / 3;
    
    QImage r = QImage( in.width(), in.height() + H, QImage::Format_ARGB32_Premultiplied );
    
	QImage in2 = in;
	in2.convertToFormat( QImage::Format_ARGB32_Premultiplied );
    
    QImage reflection = in.copy( 0, in.height() - H, in.width(), H );
    reflection = reflection.mirrored( false, true /*vertical only*/ );
    
    QPainter p( &r );
    p.drawImage( 0, 0, in2 );
    p.drawImage( 0, in.height(), reflection );

    QLinearGradient g( QPointF( 0, 0 ), QPointF( 0, 1 ) );
    g.setCoordinateMode( QGradient::ObjectBoundingMode );
    g.setColorAt( 0, QColor(0, 0, 0, 100) );
    g.setColorAt( 1, Qt::transparent );

    p.setRenderHint( QPainter::Antialiasing );
    p.setRenderHint( QPainter::SmoothPixmapTransform );

    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    p.fillRect( QRectF( QPointF( 0, in.height() ), QSizeF( r.width(), H ) ), g );
    return r;
}


void
ImageFucker::onImageDownloaded()
{
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
    QByteArray const data = reply->readAll();

    QImage in;
    in.loadFromData( data );

    height = in.height();
    pixmap = reflect0rize( in );

    y = rand() % 1000;

    emit fucked();
}

