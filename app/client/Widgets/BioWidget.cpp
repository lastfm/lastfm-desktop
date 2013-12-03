/*
   Copyright 2011 Last.fm Ltd.
      - Primarily authored by Jono Cole and Michael Coffey

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

#include <QEventLoop>
#include <QApplication>
#include <QToolTip>
#include <QTimer>
#include <QDebug>

#include "lib/unicorn/widgets/BannerWidget.h"
#include "lib/unicorn/widgets/HttpImageWidget.h"
#include "lib/unicorn/DesktopServices.h"

#include "../Services/AnalyticsService.h"
#include "../Application.h"
#include "WidgetTextObject.h"
#include "BioWidget.h"

BioWidget::BioWidget( QWidget* p ) 
    :QTextBrowser( p ),
      m_currentHoverWidget(0)
{
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    connect(document()->documentLayout(), SIGNAL( documentSizeChanged(QSizeF)), SLOT( onBioChanged(QSizeF)));

    connect(this, SIGNAL(anchorClicked(QUrl)), SLOT(onAnchorClicked(QUrl)));

    m_widgetTextObject = new WidgetTextObject;
    viewport()->installEventFilter( this );
    document()->documentLayout()->registerHandler( WidgetImageFormat, m_widgetTextObject );

    ui.image = new HttpImageWidget(this);
    ui.image->setFixedWidth( 160 );
    ui.image->setAlignment( Qt::AlignTop );
    
    ui.onTour = new BannerWidget( tr("On Tour" ) );
    ui.onTour->setBannerVisible( false );
    ui.onTour->setWidget( ui.image );

    ui.onTour->setFixedWidth( 170 );
    ui.onTour->setObjectName( "onTour" );

    connect( ui.image, SIGNAL(loaded()), SLOT(onImageLoaded()));
    connect( ui.image, SIGNAL(loaded()), SLOT(update()));

    connect( this, SIGNAL(highlighted(QString)), SLOT(onHighlighted(QString)) );
}

BioWidget::~BioWidget()
{
    delete m_widgetTextObject;
}

void
BioWidget::setBioText( const QString& bioText )
{
    m_bioText = bioText;
}

void
BioWidget::onHighlighted( const QString& url )
{
    QUrl displayUrl( url );
    QToolTip::showText( cursor().pos(), displayUrl.toString(), this, QRect() );
}

void
BioWidget::onImageLoaded()
{
    insertWidget( ui.onTour );

    append( m_bioText );

    updateGeometry();
    emit finished();
}


void 
BioWidget::insertWidget( QWidget* w ) 
{
    w->installEventFilter( this );
    m_widgetImageFormat.setObjectType( WidgetImageFormat );
    m_widgetImageFormat.setProperty( WidgetData, QVariant::fromValue<QWidget*>( w ) );
    m_widgetImageFormat.setName( w->objectName() );
    
    QTextCursor cursor = textCursor();
    cursor.insertImage( m_widgetImageFormat, QTextFrameFormat::FloatLeft );
    setTextCursor( cursor );
}

bool 
BioWidget::eventFilter( QObject* o, QEvent* e )
{
    QWidget* w = qobject_cast<QWidget*>( o );

    if ( viewport() == w )
    {
        if ( QEvent::MouseMove != e->type() )
            return false;

        QMouseEvent* event = static_cast<QMouseEvent*>(e);
        //respect child widget cursor
        QWidget* w = m_widgetTextObject->widgetAtPoint(event->pos() );

        if ( w != m_currentHoverWidget )
        {
            m_currentHoverWidget = w;

            if( 0 == w )
                viewport()->unsetCursor();
            else
            {
                QWidget* c = w->childAt(event->pos());
                c = c ? c : w;
                viewport()->setCursor( c->cursor());
            }
        }

        return false;
    }
    return false;
}


void
BioWidget::mousePressEvent( QMouseEvent* event )
{
    update();
    if ( !sendMouseEvent(event) )
        QTextBrowser::mousePressEvent( event );
}


void
BioWidget::mouseReleaseEvent( QMouseEvent* event )
{
    if ( !sendMouseEvent(event) )
        QTextBrowser::mouseReleaseEvent( event );
}


void
BioWidget::mouseMoveEvent( QMouseEvent* event ) 
{
    if (!sendMouseEvent(event))
        QTextBrowser::mouseMoveEvent( event );
}

bool 
BioWidget::sendMouseEvent( QMouseEvent* event ) 
{
    QWidget* w = m_widgetTextObject->widgetAtPoint( event->pos());

    if ( !w )
        return false;

    QRectF wRect = m_widgetTextObject->widgetRect( w );
    QPoint pos = event->pos() - wRect.toRect().topLeft();

    QWidget* childWidget = w->childAt( event->pos());

    if( !childWidget )
        childWidget = w;
    else
        pos = childWidget->mapTo( w, pos );

    QMouseEvent* widgetMouseEvent = new QMouseEvent( event->type(), pos, event->button(), event->buttons(), event->modifiers());

    QCoreApplication::postEvent( childWidget, widgetMouseEvent );
    event->accept();

    return true;
}

void
BioWidget::showEvent( QShowEvent* /*event*/ )
{
    // HACK: onBioChanged reports the wrong size for the document
    // and id we polish it a bit later it gets set correctly.
    // Polish very soon after being shown so that it gets
    // the right height quickly if you're looking at it.
    // Do another polish a bit later as for the case when you
    // switch to that tab as the first one won't work.
    QTimer::singleShot( 1, this, SLOT(polish()));
    QTimer::singleShot( 200, this, SLOT(polish()));
}

void 
BioWidget::onAnchorClicked( const QUrl& link )
{
    unicorn::DesktopServices::openUrl( link );
    AnalyticsService::instance().sendEvent( aApp->currentCategory(), LINK_CLICKED, "BioPageLinkClicked");
}


void
BioWidget::onDocumentLayoutChanged()
{
    setFixedHeight( document()->size().height() );
}

void 
BioWidget::onBioChanged( const QSizeF& /*size*/ )
{
    updateGeometry();

    onDocumentLayoutChanged();
}

void
BioWidget::polish()
{
    if ( isVisible() )
        style()->polish( this );
    else
        QTimer::singleShot( 20, this, SLOT(polish()));
}

void
BioWidget::setPixmap( const QPixmap& pixmap )
{
    ui.image->setPixmap( pixmap );
}

void 
BioWidget::loadImage( const QUrl& url, HttpImageWidget::ScaleType scale )
{
    ui.image->loadUrl( url, scale );
}

void
BioWidget::setImageHref( const QUrl& href )
{
    ui.image->setHref( href );
}

void 
BioWidget::setOnTourVisible( bool visible, const QUrl& url )
{
    ui.onTour->setBannerVisible( visible );

    if ( url.isValid())
        ui.onTour->setHref( url );

    update();
}
