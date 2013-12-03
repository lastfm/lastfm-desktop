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

#include <QWidget>

#include "WidgetTextObject.h"

WidgetTextObject::WidgetTextObject()
    :kMargin( 10, 5 )
{
}

QSizeF
WidgetTextObject::intrinsicSize(QTextDocument*, int /*posInDocument*/, const QTextFormat& format)
{
    QWidget* widget = qVariantValue<QWidget*>(format.property(1));
    return QSizeF( widget->size() + kMargin );
}

void
WidgetTextObject::drawObject(QPainter *painter, const QRectF &rect, QTextDocument * /*doc*/, int /*posInDocument*/, const QTextFormat &format)
{
    QWidget* widget = qVariantValue<QWidget*>(format.property( 1 ));
    widget->render( painter, QPoint( 0, 0 ));

    //Adjusted to allow for the margin
    QRect contentsRect = rect.toRect().adjusted(0, 0, -kMargin.width(), -kMargin.height());
    m_widgetRects[widget] = contentsRect;
}

QWidget*
WidgetTextObject::widgetAtPoint( const QPoint& p )
{
    QMapIterator<QWidget*, QRect> i(m_widgetRects);
    while (i.hasNext())
    {
        i.next();
        if( i.value().contains(p))
            return i.key();
    }
    return 0;
}

QRectF
WidgetTextObject::widgetRect( QWidget* w )
{
    return m_widgetRects[w];
}
