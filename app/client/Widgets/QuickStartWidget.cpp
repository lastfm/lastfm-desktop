/*
   Copyright 2005-2009 Last.fm Ltd. 

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

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPoint>
#include <QMenu>
#include <QShortcut>
#include <QCompleter>
#include <lastfm/Tag.h>
#include <lastfm/User.h>
#include <lastfm/Chart.h>
#include <lastfm/XmlQuery.h>

#include "lib/unicorn/widgets/Label.h"

#include "QuickStartWidget.h"
#include "../Application.h"
#include "../Services/AnalyticsService.h"

#include <QStylePainter>

#define RESULT_LIMIT 10

QuickStartWidget::QuickStartWidget( QWidget* parent )
    :QFrame( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );

    QHBoxLayout* h = new QHBoxLayout();
    h->setContentsMargins( 0, 0, 0, 0 );
    h->setSpacing( 0 );

    layout->addLayout( h );

    h->addWidget( ui.edit = new QLineEdit( this ) );
#if QT_VERSION >= 0x040700
    // The placeholder property was introduced in Qt 4.7
    ui.edit->setPlaceholderText( tr("Type an artist or tag and press play") );
#endif
    ui.edit->setAttribute( Qt::WA_MacShowFocusRect, false );

    connect( ui.edit, SIGNAL(returnPressed()), SLOT(play()));

    h->addWidget( ui.button = new QPushButton( tr("Play"), this ) );
    ui.button->setToolTip( tr( "Play" ) );
    ui.button->setContextMenuPolicy( Qt::CustomContextMenu );
    ui.button->setEnabled( true );

    connect( ui.button, SIGNAL(clicked()), SLOT(play()));
    connect( ui.button, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customContextMenuRequested(QPoint)));

    layout->addWidget( ui.whyNotTry = new Label( "", this ) );
    ui.whyNotTry->setObjectName( "whyNotTry" );
    ui.whyNotTry->setTextFormat( Qt::RichText );
    ui.whyNotTry->setWordWrap( true );

    ui.button->setAttribute( Qt::WA_MacNoClickThrough );

    QShortcut* shortcut = new QShortcut( ui.edit );
    shortcut->setKey( Qt::CTRL + Qt::Key_D );
    shortcut->setContext( Qt::WidgetShortcut );
    connect( shortcut, SIGNAL(activated()), SLOT(setToCurrent()) );

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );

    onSessionChanged( aApp->currentSession() );
}

void
QuickStartWidget::onSessionChanged( const unicorn::Session& session )
{
    if ( session.user().name() != m_currentUser )
    {
        m_currentUser = session.user().name();

        m_tags.clear();
        m_artists.clear();
        ui.whyNotTry->clear();

        connect( session.user().getTopArtists( "3month", RESULT_LIMIT ), SIGNAL(finished()), SLOT(onUserGotTopArtists()) );
    }
}

void
QuickStartWidget::onGotTagSuggestions()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( static_cast<QNetworkReply*>(sender()) ) )
    {
        foreach ( const XmlQuery& suggestion, lfm["suggestions"].children( "suggestion" ) )
            m_tags << suggestion["tag"]["name"].text();

        // If we don't have enough to display, get Last.fm's overall top tags
        if ( m_tags.count() < 2 )
            connect( lastfm::Tag::getTopTags(), SIGNAL(finished()), SLOT(onGotTopTags()) );

        setSuggestions();
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}

void
QuickStartWidget::onGotTopTags()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( static_cast<QNetworkReply*>(sender()) ) )
    {
        foreach ( const XmlQuery& tag, lfm["toptags"].children( "tag" ) )
            m_tags << tag["name"].text();

        setSuggestions();
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}

void
QuickStartWidget::onUserGotTopArtists()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( static_cast<QNetworkReply*>(sender()) ) )
    {
        foreach ( const XmlQuery& artist, lfm["topartists"].children( "artist" ) )
            m_artists << artist["name"].text();

        if ( m_artists.count() < 2 )
            connect( Chart::getTopArtists( RESULT_LIMIT ), SIGNAL(finished()), SLOT(onChartGotTopArtists()) );

        setSuggestions();
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}

void
QuickStartWidget::onChartGotTopArtists()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( static_cast<QNetworkReply*>(sender()) ) )
    {
        foreach ( const XmlQuery& artist, lfm["artists"].children( "artist" ) )
            m_artists << artist["name"].text();

        setSuggestions();
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}
