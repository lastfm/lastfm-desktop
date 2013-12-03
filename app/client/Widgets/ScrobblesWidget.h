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

#ifndef RECENT_TRACKS_WIDGET_H_
#define RECENT_TRACKS_WIDGET_H_

#include <QWidget>

namespace lastfm { class Track; }
using lastfm::Track;

namespace Ui { class ScrobblesWidget; }

class ScrobblesWidget : public QWidget
{
Q_OBJECT
public:
    explicit ScrobblesWidget( QWidget* parent = 0 );
    ~ScrobblesWidget();

public slots:
    void onCurrentChanged( int index );
    void refresh();

protected slots:
    void onTrackClicked( class TrackWidget& trackWidget );
    void onBackClicked();
    void onMoveFinished();
    void onMetadataWidgetFinished();

protected:
    Ui::ScrobblesWidget* ui;
    int m_lastIndex;
};

#endif //RECENT_TRACKS_WIDGET_H_
