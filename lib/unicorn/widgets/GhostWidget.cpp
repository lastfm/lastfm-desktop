/*
   Copyright 2010-2011 Last.fm Ltd.
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

#include "GhostWidget.h"
#include <QResizeEvent>
#include <QDebug>

GhostWidget::GhostWidget( QWidget* parent )
    :QWidget( parent )
{
}

void
GhostWidget::setOrigin( QWidget* origin )
{
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    setFixedSize( origin->sizeHint() );
    setVisible( origin->isVisible() );
    origin->installEventFilter( this );
}

bool 
GhostWidget::eventFilter( QObject* /*obj*/, QEvent* event )
{
    switch ( event->type() )
    {
    case QEvent::Resize:
        {
        QResizeEvent* re = static_cast<QResizeEvent*>( event );
        setFixedSize( re->size() );
        }
        break;
    case QEvent::Show:
        show();
        break;
    case QEvent::Hide:
        hide();
        break;
    default:
        break;
    }

    return false;
}

