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

#ifndef SLIDINGSTACKEDWIDGET_H
#define SLIDINGSTACKEDWIDGET_H

#include <QStackedWidget>
#include <QEasingCurve>

#include "lib/DllExportMacro.h"

namespace unicorn {

class UNICORN_DLLEXPORT SlidingStackedWidget : public QStackedWidget
{
        Q_OBJECT

public:
        //! This enumeration is used to define the animation direction
        enum t_direction {
                LEFT2RIGHT,
                RIGHT2LEFT,
                TOP2BOTTOM,
                BOTTOM2TOP,
                AUTOMATIC
        };

        //! The Constructor and Destructor
        explicit SlidingStackedWidget(QWidget *parent);

        Q_PROPERTY( QEasingCurve::Type easingCurve READ easingCurve WRITE setEasingCurve )
        Q_PROPERTY( int speed READ speed WRITE setSpeed )

        QEasingCurve::Type easingCurve() const { return m_animationtype; }
        void setEasingCurve( QEasingCurve::Type animationtype ) { m_animationtype = animationtype; }

        int speed() const { return m_speed; }
        void setSpeed( int speed ) { m_speed = speed; }

public slots:
        //! Some basic settings API
        void setAnimation(enum QEasingCurve::Type animationtype); //check out the QEasingCurve documentation for different styles

        //! The Animation / Page Change API
        void slide( int index  );

signals:
        //! this is used for internal purposes in the class engine
        void animationFinished(void);

protected slots:
        //! this is used for internal purposes in the class engine
        void animationDoneSlot(void);

protected:
        //! this is used for internal purposes in the class engine
        void slideWidget( QWidget * widget );

        QWidget* m_mainwindow;

        int m_speed;
        enum QEasingCurve::Type m_animationtype;
        bool m_vertical;
        int m_now;
        int m_next;
        QPoint m_pnow;
        bool m_active;

        int m_index;
};

}

#endif // SLIDINGSTACKEDWIDGET_H
