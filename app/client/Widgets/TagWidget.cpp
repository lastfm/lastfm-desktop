/*
   Copyright 2011 Last.fm Ltd.
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

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QPainter>
#include <QFontMetrics>
#include <QEvent>

#include "../Application.h"
#include "lib/unicorn/widgets/Label.h"
#include "lib/unicorn/DesktopServices.h"
#include "../Services/AnalyticsService.h"

#include "TagWidget.h"

TagWidget::TagWidget( const QString& tag, const QString& url, QWidget *parent )
    :QPushButton( tag, parent ),
      m_url( url ),
      m_hovered( false ),
      m_left_rest( ":/meta_tag_LEFT_REST.png" ),
      m_middle_rest( ":/meta_tag_MIDDLE_REST.png" ),
      m_right_rest( ":/meta_tag_RIGHT_REST.png" ),
      m_left_hover( ":/meta_tag_LEFT_HOVER.png" ),
      m_middle_hover( ":/meta_tag_MIDDLE_HOVER.png" ),
      m_right_hover( ":/meta_tag_RIGHT_HOVER.png" )
{    
    connect( this, SIGNAL(clicked()), SLOT(onClicked()));

    setAttribute( Qt::WA_LayoutUsesWidgetRect );
    setAttribute( Qt::WA_Hover );

    this->setCursor( Qt::PointingHandCursor );
}

bool
TagWidget::event( QEvent* e )
{
    switch ( e->type())
    {
    case QEvent::HoverEnter:
        m_hovered = true;
        update();
        break;
    case QEvent::HoverLeave:
        m_hovered = false;
        update();
        break;
    default:
        break;
    }

    return QPushButton::event( e );
}

void
TagWidget::paintEvent( QPaintEvent* /*event*/ )
{
    QPainter p( this );

    QFontMetrics fm = fontMetrics();

    QRect middleRect = rect();
    middleRect.adjust( m_left_rest.width(), 0, 0, 0 );
    middleRect.setWidth( fm.width( text() ) + 8 );

    if ( !m_hovered )
    {
        p.drawPixmap( rect().topLeft(), m_left_rest );
        p.drawPixmap( middleRect, m_middle_rest );
        p.drawPixmap( QPoint( m_left_rest.width() + middleRect.width(), 0 ), m_right_rest );
    }
    else
    {
        p.drawPixmap( rect().topLeft(), m_left_hover );
        p.drawPixmap( middleRect, m_middle_hover );
        p.drawPixmap( QPoint( m_left_hover.width() + middleRect.width(), 0 ), m_right_hover );
    }

    QTextOption to;
    to.setAlignment( Qt::AlignCenter );

    p.drawText( middleRect.adjusted( 0, 0, 0, -1 ), text(), to );
}

QSize
TagWidget::sizeHint() const
{
    QFontMetrics fm = fontMetrics();
    QSize size = QPushButton::sizeHint();
    size.setWidth( m_left_rest.width() + m_right_rest.width() + fm.width( text() ) + 8 );
    size.setHeight( 19 );
    return size;
}

void
TagWidget::onClicked()
{
    unicorn::DesktopServices::openUrl( m_url );
    AnalyticsService::instance().sendEvent( aApp->currentCategory(), TAG_CLICKED, "TagButtonPressed");

}
