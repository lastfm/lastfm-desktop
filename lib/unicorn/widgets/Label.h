/*
   Copyright 2011-2013 Last.fm Ltd.
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

#ifndef LABEL_H
#define LABEL_H

#include <QLabel>

#include "lib/DllExportMacro.h"

namespace unicorn
{

class UNICORN_DLLEXPORT Label : public QLabel
{
    Q_OBJECT
public:
    explicit Label( QWidget* parent = 0 );
    explicit Label( const QString& text, QWidget* parent = 0 );

    QString text() const;
    void setText( const QString& text );
    void setLinkColor( QColor linkColor );

    static QString anchor( const QString& url, const QString& text );
    static QString boldLinkStyle( const QString& text, QColor linkColor );

    // Gives you a pretty time string and will call your slot when it's time to change it again
    static void prettyTime( Label& timestampLabel, const class QDateTime& timestamp, QTimer* callback = 0 );
    static QString price( const QString& price, const QString& currency );

private:
    void paintEvent( QPaintEvent* event );

    QSize sizeHint() const;

    QString boldLinkStyle( const QString& text );

private slots:
    void onHovered( const QString& url );
    void onActivated( const QString& url );

private:
    QString m_text;
    QColor m_linkColor;
};

}

#endif // LABEL_H
