/*
   Copyright 2010-2012 Last.fm Ltd.
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

#include "BannerWidget.h"

#include <QStackedLayout>
#include <QDesktopServices>
#include <QMoveEvent>
#include <QDebug>
#include <QCoreApplication>

#include "lib/unicorn/DesktopServices.h"

BannerWidget::BannerWidget( const QString& pText, QWidget* parent )
    :QFrame( parent ), m_childWidget( 0 )
{
    m_layout = new QStackedLayout( this );
    setLayout( m_layout );
    m_layout->setStackingMode( QStackedLayout::StackAll );
    m_layout->addWidget( m_banner = new BannerWidgetPrivate(pText) );
    connect( m_banner, SIGNAL( clicked() ), this, SLOT( onClick() ) );
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

void 
BannerWidget::setWidget( QWidget* w )
{
    //Remove any existing childWidget
    if( m_layout->count() > 1 )
    {
        m_childWidget->removeEventFilter( this );
        m_layout->removeWidget( m_childWidget );
    }

    m_childWidget = w;
    m_childWidget->installEventFilter( this );
    m_layout->insertWidget( 0, m_childWidget );
    m_layout->setCurrentWidget( m_banner );
}

void 
BannerWidget::setHref( const QUrl& url )
{
#ifdef Q_OS_MAC
    //On OSX Percent encoding seems to get applied to the url again.
    m_href = QUrl::fromPercentEncoding( url.toString().toUtf8() );
#else
    m_href = url.toString();
#endif

    if( url.isValid())
        setCursor( Qt::PointingHandCursor );
    else
        unsetCursor();
}

void
BannerWidget::onClick()
{
    unicorn::DesktopServices::openUrl( m_href );
}

QSize 
BannerWidget::sizeHint() const
{
    if( m_childWidget )
        return m_childWidget->sizeHint();

    return QWidget::sizeHint();
}

void 
BannerWidget::mousePressEvent( QMouseEvent* e )
{
    QCoreApplication::sendEvent( m_layout, e );
}

bool
BannerWidget::eventFilter( QObject* o, QEvent* e )
{
    QWidget* w = qobject_cast<QWidget*>(o);

    if( !w )
        return false;

    if( e->type() == QEvent::Resize )
        resize( static_cast<QResizeEvent*>(e)->size() );

    return false;
}

void 
BannerWidget::setBannerVisible( bool visible )
{
    m_banner->setVisible( visible );
}

bool 
BannerWidget::bannerVisible() const
{
    return m_banner->isVisible();
}


BannerWidgetPrivate::BannerWidgetPrivate( const QString& pText, QWidget* parent )
    :QAbstractButton(parent)
{
    setText( QString( " " ) + pText + " " );
}

void
BannerWidgetPrivate::paintEvent( QPaintEvent* /*e*/ )
{
    QPainter painter( this );

    painter.setRenderHint( QPainter::TextAntialiasing );
    painter.setRenderHint( QPainter::Antialiasing );

    QRect bgRect = m_textRect.adjusted( -20, 0, 20, 0 );

    painter.setWorldMatrix( m_transformMatrix );

    painter.fillRect( bgRect, palette().brush( QPalette::Window ));
    style()->drawItemText( &painter, m_textRect.translated( 0, -1 ), Qt::AlignCenter, palette(), true, text() );
}

void
BannerWidgetPrivate::resizeEvent( QResizeEvent* event )
{
    clearMask();
    QFont f = font();
    m_textRect = QFontMetrics( f ).boundingRect( text() );
    m_textRect.adjust( 0, 0, 0, 5 );
    m_transformMatrix.reset();

    //Tiny optimization and means math.h doesn't need to be included
    //and saves some runtime ops. I shouldn't imagine sin(45) is likely to change anytime soon!
    const float sin45 = 0.707106781186548f;

    m_transformMatrix.translate( event->size().width() - ((sin45 * m_textRect.width()) + 6 ), (sin45 * m_textRect.height()) - 6 );
    m_transformMatrix.rotate( 45 );

    QRegion mask = m_transformMatrix.map( QRegion( m_textRect.adjusted( -20, 0, 20, 0 ) ) );
    setMask( mask );
}

void
BannerWidgetPrivate::mousePressEvent( QMouseEvent* e )
{
    if( !mask().contains( e->pos() ) )
    {
        e->ignore();
        return;
    }

    e->accept();
    return QAbstractButton::mousePressEvent( e );
}

void
BannerWidgetPrivate::mouseReleaseEvent( QMouseEvent* e )
{
    if( !mask().contains( e->pos() ) )
    {
        e->ignore();
        return;
    }

    e->accept();
    return QAbstractButton::mouseReleaseEvent( e );
}
