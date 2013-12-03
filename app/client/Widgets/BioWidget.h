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

#ifndef BIO_WIDGET_H_
#define BIO_WIDGET_H_

#include <QTextBrowser>
#include <QTextObjectInterface>
#include <QDebug>
#include "lib/unicorn/widgets/HttpImageWidget.h"

/** A specialized QTextBrowser which can insert widgets inline */
#include <QPlainTextDocumentLayout>
class BioWidget : public QTextBrowser
{
    Q_OBJECT
public:
    BioWidget( QWidget* parent );
    ~BioWidget();

    bool eventFilter( QObject* o, QEvent* e );
    
    void setBioText( const QString& bioText );

    void setPixmap( const QPixmap& pixmap );
    void loadImage( const QUrl&, HttpImageWidget::ScaleType scale = HttpImageWidget::ScaleAuto );
    void setImageHref( const QUrl& );
    
    void setOnTourVisible( bool, const QUrl& = QUrl());

signals:
    void finished();

protected slots:
    void onBioChanged( const QSizeF& size );
    void onAnchorClicked( const QUrl& link );

    void onHighlighted( const QString& url );

    void onImageLoaded();

    void onDocumentLayoutChanged();
    void polish();

protected:
    void insertWidget( QWidget* w );
    class WidgetTextObject* m_widgetTextObject;

    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void showEvent(QShowEvent *);

    bool sendMouseEvent( QMouseEvent* event );

    enum WidgetProperties { WidgetData = 1 };
    enum { WidgetImageFormat = QTextFormat::UserObject + 1 };
    QWidget* m_currentHoverWidget;

    QString m_bioText;

    QTextImageFormat m_widgetImageFormat;

    struct {
        class BannerWidget* onTour;
        class HttpImageWidget* image;
    } ui;
};


#endif //BIO_WIDGET_H_
