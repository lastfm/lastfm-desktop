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

#include <QEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QToolTip>
#include <QUrl>
#include <QGraphicsDropShadowEffect>
#include <QDateTime>
#include <QTimer>

#include "../DesktopServices.h"

#include "Label.h"

unicorn::Label::Label( QWidget* parent )
    :QLabel( parent ), m_linkColor( QRgb( 0x333333 ) )
{
    setAttribute( Qt::WA_LayoutUsesWidgetRect );
    setAttribute( Qt::WA_MacNoClickThrough );

    setOpenExternalLinks( false );

    connect( this, SIGNAL(linkHovered(QString)), SLOT(onHovered(QString)));
    connect( this, SIGNAL(linkActivated(QString)), SLOT(onActivated(QString)));
}


unicorn::Label::Label( const QString& text, QWidget* parent )
    :QLabel( parent )
{
    setText( text );
    setAttribute( Qt::WA_LayoutUsesWidgetRect );
    setAttribute( Qt::WA_MacNoClickThrough );
    setOpenExternalLinks( false );

    connect( this, SIGNAL(linkHovered(QString)), SLOT(onHovered(QString)));
    connect( this, SIGNAL(linkActivated(QString)), SLOT(onActivated(QString)));
}

void
unicorn::Label::onActivated( const QString& url )
{
    unicorn::DesktopServices::openUrl( url );
}

void
unicorn::Label::onHovered( const QString& url )
{   
    QUrl displayUrl( url );
    QToolTip::showText( cursor().pos(), displayUrl.toString(), this, QRect() );
}

QString
unicorn::Label::boldLinkStyle( const QString& text, QColor linkColor )
{
    return QString( "<html><head><style type=text/css>"
                     "a:link {color:%1; font-weight: bold; text-decoration:none;}"
                     "a:hover {color:%1; font-weight: bold; text-decoration:none;}"
                     "</style></head><body>%2</body></html>" ).arg( linkColor.name(), text );
}

QString
unicorn::Label::boldLinkStyle( const QString& text )
{
    return boldLinkStyle( text, m_linkColor );
}

QString
unicorn::Label::text() const
{
    return m_text;
}

void
unicorn::Label::setText( const QString& text )
{
    m_text = text;

    if ( textFormat() == Qt::RichText )
        QLabel::setText( boldLinkStyle( m_text ) );
    else
        QLabel::setText( ""  );

    update();
}

void
unicorn::Label::setLinkColor( QColor linkColor )
{
    m_linkColor = linkColor;
}

QString
unicorn::Label::anchor( const QString& url, const QString& text )
{
    QString actualText = text;
    actualText.replace( QRegExp( "&" ), "&amp;" ); // This _must_ come first
    actualText.replace( QRegExp( "<" ), "&lt;" );
    actualText.replace( QRegExp( ">" ), "&gt;" );
    actualText.replace( QRegExp( "\"" ), "&quot;" );

    return QString( "<a href=\"%1\">%2</a>" ).arg( url, actualText );
}

void
unicorn::Label::prettyTime( Label& timestampLabel, const QDateTime& timestamp, QTimer* callback )
{
    QDateTime now = QDateTime::currentDateTime();

    // Full time in the tool tip
    timestampLabel.setToolTip( timestamp.toString( Qt::DefaultLocaleLongDate ) );

    int secondsAgo = timestamp.secsTo( now );

    if ( secondsAgo < (60 * 60) )
    {
        // Less than an hour ago
        int minutesAgo = ( timestamp.secsTo( now ) / 60 );
        timestampLabel.setText( tr( "%n minute(s) ago", "", minutesAgo ) );
        if ( callback ) callback->start( now.secsTo( timestamp.addSecs(((minutesAgo + 1 ) * 60 ) + 1 ) ) * 1000 );
    }
    else if ( secondsAgo < (60 * 60 * 6) || now.date() == timestamp.date() )
    {
        // Less than 6 hours ago or on the same date
        int hoursAgo = ( timestamp.secsTo( now ) / (60 * 60) );
        timestampLabel.setText( tr( "%n hour(s) ago", "", hoursAgo ) );
        if ( callback ) callback->start( now.secsTo( timestamp.addSecs( ( (hoursAgo + 1) * 60 * 60 ) + 1 ) ) * 1000 );
    }
    else if ( secondsAgo < (60 * 60 * 24 * 365) )
    {
        // less than a year ago
        timestampLabel.setText( timestamp.toString( Qt::DefaultLocaleShortDate ) );
        // We don't need to set the timer because this date will never change (well, it might in a year's time)
    }
    else
    {
        timestampLabel.setText( timestamp.toString( Qt::DefaultLocaleLongDate ) );
        // We don't need to set the timer because this date will never change
    }

    if ( secondsAgo < 0 )
        timestampLabel.setText( tr( "Time is broken" ) ); // in the future!
}

QString
unicorn::Label::price( const QString& price, const QString& currency )
{
    QString returnPrice;

    if ( currency.compare( "MXN", Qt::CaseInsensitive ) == 0
              || currency.compare( "USD", Qt::CaseInsensitive ) == 0
              || currency.compare( "AUD", Qt::CaseInsensitive ) == 0
              || currency.compare( "NZD", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "$%1" ).arg( price );
    else if ( currency.compare( "CAD", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "CAD$%1" ).arg( price );
    else if ( currency.compare( "DKK", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "DKK%1" ).arg( price );
    else if ( currency.compare( "CHF", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "CHF%1" ).arg( price );
    else if ( currency.compare( "NOK", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "NOK%1" ).arg( price );
    else if ( currency.compare( "SEK", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "SEK%1" ).arg( price );
    else if ( currency.compare( "GBP", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "£%1" ).arg( price );
    else if ( currency.compare( "EUR", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "€%1" ).arg( price );
    else if ( currency.compare( "JPY", Qt::CaseInsensitive ) == 0 )
        returnPrice = QString::fromUtf8( "¥%1" ).arg( price );
    else
        returnPrice = QString( "%1 %2" ).arg( price, currency );

    return returnPrice;
}

QSize
unicorn::Label::sizeHint() const
{
    QSize sizeHint = QLabel::sizeHint();

    if ( textFormat() != Qt::RichText )
        sizeHint.setWidth( qMin ( sizeHint.width(), fontMetrics().width( m_text ) + 1 ) );

    return sizeHint;
}

void
unicorn::Label::paintEvent( QPaintEvent* event )
{
    if ( textFormat() == Qt::RichText )
        QLabel::paintEvent( event );
    else
    {
        QFrame::paintEvent(event);
        QPainter p(this);
        p.drawText( rect(), fontMetrics().elidedText( m_text, Qt::ElideRight, contentsRect().width() ) );
    }
}


