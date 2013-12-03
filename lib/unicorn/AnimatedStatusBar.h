/*
   Copyright 2009 Last.fm Ltd.
      - Primarily authored by Jono Cole

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

#ifndef ANIMATED_STATUS_BAR_H
#define ANIMATED_STATUS_BAR_H

#include <lib/DllExportMacro.h>
#include <QStatusBar>

class UNICORN_DLLEXPORT AnimatedStatusBar: public QStatusBar {
    Q_OBJECT
public:
    AnimatedStatusBar( QWidget* parent = 0 );

public slots:
    void showAnimated();
    void hideAnimated();

private slots:
    void onFrameChanged(int);
    void onFinished();

private:
    class QTimeLine* m_timeline;
    int m_windowHeight;
};

#endif //ANIMATED_STATUS_BAR_H
