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

#ifndef BANNER_WIDGET_H
#define BANNER_WIDGET_H

#include <QFrame>
#include <QMatrix>
#include <QRect>
#include <QUrl>
#include <QStyle>
#include <QResizeEvent>
#include <QPainter>
#include <QAbstractButton>

#include "lib/DllExportMacro.h"
#include <QDebug>

class UNICORN_DLLEXPORT BannerWidget : public QFrame
{
    Q_OBJECT
public:
    BannerWidget( const QString& text, QWidget* parent = 0 );
    bool bannerVisible() const;
    void setWidget( QWidget* w );
    QSize sizeHint() const;

public slots:
    void setHref( const QUrl& url );
    void setBannerVisible( bool visible = true );

protected:
    void mousePressEvent( QMouseEvent* e );
    bool eventFilter( QObject*, QEvent* );

private slots:
    void onClick();

private:
    class QStackedLayout* m_layout;
    class BannerWidgetPrivate* m_banner;
    bool m_bannerVisible;

    QString m_href;
    QWidget* m_childWidget;
};



class BannerWidgetPrivate : public QAbstractButton
{
    Q_OBJECT
public:
    BannerWidgetPrivate( const QString& pText, QWidget* parent = 0 );

private:
    void paintEvent( QPaintEvent* /*e*/ );
    void resizeEvent( QResizeEvent* event );
    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );

private:
    QMatrix m_transformMatrix;
    QRect m_textRect;
};

#endif //BANNER_WIDGET_H

