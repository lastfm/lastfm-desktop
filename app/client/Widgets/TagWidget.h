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

#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPushButton>
#include <QUrl>

class TagWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit TagWidget( const QString& tag, const QString& url, QWidget *parent = 0);

private:
    void paintEvent( QPaintEvent* event );
    QSize sizeHint() const;
    bool event( QEvent* e );

private slots:
    void onClicked();

private:
    QUrl m_url;

    bool m_hovered;

    QPixmap m_left_rest;
    QPixmap m_middle_rest;
    QPixmap m_right_rest;

    QPixmap m_left_hover;
    QPixmap m_middle_hover;
    QPixmap m_right_hover;
};

#endif // TAGWIDGET_H
