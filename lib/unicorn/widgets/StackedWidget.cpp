/*
   Copyright 2012 Last.fm Ltd.
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

#include "StackedWidget.h"

unicorn::StackedWidget::StackedWidget(QWidget *parent) :
    QStackedWidget(parent)
{
    connect( this, SIGNAL(currentChanged(int)), SLOT(onCurrentChanged(int)));
}

QSize
unicorn::StackedWidget::sizeHint() const
{
    return currentWidget()->sizeHint();
}

void
unicorn::StackedWidget::onCurrentChanged( int index )
{
    for ( int i = 0 ; i < count() ; ++i )
    {
        if ( i == index )
        {
            widget( i )->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
            widget( i )->adjustSize();
        }
        else
            widget( i )->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    }

    adjustSize();
}
