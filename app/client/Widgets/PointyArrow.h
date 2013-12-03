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

#ifndef POINTY_ARROW_H
#define POINTY_ARROW_H

#include <QWidget>
#include <QPixmap>

class PointyArrow : public QWidget {
    Q_OBJECT

public:
    PointyArrow();
    ~PointyArrow();

    void pointAt( const QPoint& );

protected:
    virtual void paintEvent( QPaintEvent* );
    QPixmap m_pm;
    class QTimeLine* m_timeline;

	enum { DirectionUp, DirectionDown, DirectionLeft, DirectionRight } m_currentDirection;

protected slots:
    void onFrameChanged( int );
    void onFinished();
};

#endif //POINTY_ARROW_H

