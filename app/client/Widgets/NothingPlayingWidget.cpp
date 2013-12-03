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

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProcess>

#include "../Application.h"

#include "QuickStartWidget.h"

#include "lib/unicorn/widgets/Label.h"

#ifdef Q_OS_WIN
#include "lib/unicorn/plugins/IPluginInfo.h"
#include "lib/unicorn/plugins/PluginList.h"
#endif

#include "NothingPlayingWidget.h"
#include "ui_NothingPlayingWidget.h"

NothingPlayingWidget::NothingPlayingWidget( QWidget* parent )
    :QFrame( parent ),
      ui( new Ui::NothingPlayingWidget )
{   
    ui->setupUi( this );

    setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->top->setAttribute( Qt::WA_LayoutUsesWidgetRect );

    ui->contents->setAttribute( Qt::WA_LayoutUsesWidgetRect );

    onSessionChanged( aApp->currentSession() );

    ui->scrobble->setText( tr( "<h2>Scrobble from your music player</h2>"
                               "<p>Start listening to some music in your media player. You can see more information about the tracks you play on the Now Playing tab.</p>") );

    ui->itunes->hide();
    ui->wmp->hide();
    ui->winamp->hide();
    ui->foobar->hide();

#if defined( Q_OS_WIN ) || defined( Q_OS_MAC )
    ui->itunes->setVisible( true ); // always show iTunes on Mac
    ui->itunes->setAttribute( Qt::WA_LayoutUsesWidgetRect );

    connect( ui->itunes, SIGNAL(clicked()), SLOT(oniTunesClicked()));

#ifndef Q_OS_MAC
    unicorn::PluginList pluginList;
    ui->itunes->setVisible( pluginList.pluginById( "itw" )->isAppInstalled() );
    ui->wmp->setVisible( pluginList.pluginById( "wmp" )->isAppInstalled() );
    ui->wmp->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->winamp->setVisible( pluginList.pluginById( "wa2" )->isAppInstalled() );
    ui->winamp->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    ui->foobar->setVisible( pluginList.pluginById( "foo3" )->isAppInstalled() );
    ui->foobar->setAttribute( Qt::WA_LayoutUsesWidgetRect );

    connect( ui->wmp, SIGNAL(clicked()), SLOT(onWMPClicked()));
    connect( ui->winamp, SIGNAL(clicked()), SLOT(onWinampClicked()));
    connect( ui->foobar, SIGNAL(clicked()), SLOT(onFoobarClicked()));
#endif
#endif

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );
}

void
NothingPlayingWidget::onSessionChanged( const unicorn::Session& session )
{
    if ( !session.user().name().isEmpty() )
        ui->top->setText( tr(  "Hello, %1!" ).arg( session.user().name() ) );

    // leave them in their current state until the sessioninfo is valid
    // this stops them getting hidden and shown again when adding a new subscriber
    if ( session.isValid() )
    {
        ui->splitter->setVisible( session.youRadio() );
        ui->quickStart->setVisible( session.youRadio() );
        ui->start->setVisible( session.youRadio() );
    }
}

#ifdef Q_OS_WIN
void
NothingPlayingWidget::startApp( const QString& app )
{
    QString mediaPlayer = QString( qgetenv( "ProgramFiles(x86)" ) ).append( app );

    if ( !QFile::exists( mediaPlayer ) )
        mediaPlayer = QString( qgetenv( "ProgramFiles" ) ).append( app );

    if ( QFile::exists( mediaPlayer ) )
    {
        mediaPlayer = QString( "\"%1\"" ).arg( mediaPlayer );
        QProcess::startDetached( mediaPlayer );
    }
}

void
NothingPlayingWidget::oniTunesClicked()
{
    startApp( "/iTunes/iTunes.exe" );
}

void
NothingPlayingWidget::onWinampClicked()
{
    startApp( "/Winamp/winamp.exe" );
}

void
NothingPlayingWidget::onWMPClicked()
{
    startApp( "/Windows Media Player/wmplayer.exe" );
}

void
NothingPlayingWidget::onFoobarClicked()
{
    startApp( "/foobar2000/foobar2000.exe" );
}
#endif



