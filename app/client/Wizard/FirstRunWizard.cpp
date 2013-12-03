
/*
   Copyright 2005-2009 Last.fm Ltd.
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

#include <QPalette>
#include <QStyle>
#include <QMovie>

#include "lib/unicorn/UnicornSettings.h"

#include "../Application.h"

#include "LoginPage.h"
#include "AccessPage.h"
#include "PluginsPage.h"
#include "PluginsInstallPage.h"
#include "BootstrapPage.h"
#include "TourMetadataPage.h"
#include "TourRadioPage.h"
#include "TourFinishPage.h"
#include "TourScrobblesPage.h"
#include "TourLocationPage.h"

#include "ui_FirstRunWizard.h"
#include "FirstRunWizard.h"

FirstRunWizard::FirstRunWizard( bool startFromTour, QWidget* parent )
    :QDialog( parent ),
      ui( new Ui::FirstRunWizard ),
      m_commitPage( false ),
      m_showWelcome( false ),
      m_bootstrapState( NoBootstrap )
{
#ifdef Q_OS_WIN32
    m_plugins = new unicorn::PluginList;
#endif

    ui->setupUi( this );

    ui->welcome->hide();

    for ( int i = 0 ; i < ui->stackedWidget->count() ; ++i )
    {
        qobject_cast<WizardPage*>(ui->stackedWidget->widget( i ))->setWizard( this );
    }

    connect( ui->next, SIGNAL(clicked()), SLOT(next()));
    connect( ui->back, SIGNAL(clicked()), SLOT(back()));
    connect( ui->skip, SIGNAL(clicked()), SLOT(skip()));
    connect( ui->finish, SIGNAL(clicked()), SLOT(accept()));

    connect( this, SIGNAL( rejected() ), this, SLOT( onRejected() ) );
    connect( this, SIGNAL( accepted() ), this, SLOT( onWizardCompleted() ) );

    connect( aApp, SIGNAL(bootstrapStarted(QString)), SLOT(onBootstrapStarted(QString)));
    connect( aApp, SIGNAL(bootstrapDone(AbstractBootstrapper::BootstrapStatus)), SLOT(onBootstrapDone(AbstractBootstrapper::BootstrapStatus)));

    if ( startFromTour )
        ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
    else
        ui->stackedWidget->setCurrentWidget( ui->loginPage );

    initializePage( ui->stackedWidget->currentWidget() );

    setFixedSize( 750, 450 );
}

FirstRunWizard::~FirstRunWizard()
{
#ifdef Q_OS_WIN32
    delete m_plugins;
#endif
}

FirstRunWizard::BootstrapState
FirstRunWizard::bootstrapState() const
{
    return m_bootstrapState;
}

AbstractBootstrapper::BootstrapStatus
FirstRunWizard::bootstrapStatus() const
{
    return m_bootstrapStatus;
}

void
FirstRunWizard::setTitle( const QString& title )
{
    ui->title->setText( title );
}

void
FirstRunWizard::setCommitPage( bool commitPage )
{
    m_commitPage = commitPage;
}

bool
FirstRunWizard::canGoBack() const
{
    return m_pages.count() > 0;
}

void
FirstRunWizard::showWelcome()
{
    m_showWelcome = true;
}

QAbstractButton*
FirstRunWizard::setButton( Button button, const QString& text )
{
    QAbstractButton* returnButton;

    switch ( button )
    {
    case CustomButton:
        returnButton = ui->custom;
        break;
    case BackButton:
        returnButton = ui->back;
        break;
    case SkipButton:
        returnButton = ui->skip;
        break;
    case NextButton:
        returnButton = ui->next;
        break;
    case FinishButton:
        returnButton = ui->finish;
        break;
    }

    returnButton->setText( text );
    returnButton->show();

    return returnButton;
}

void
FirstRunWizard::next()
{
    setUpdatesEnabled( false );

    QWidget* currentPage = ui->stackedWidget->currentWidget();

    if ( qobject_cast<WizardPage*>(currentPage)->validatePage() )
    {
        // go to the next page
        cleanupPage( currentPage );

        // remember what the last page was so we can go back()
        m_pages << currentPage;

        if ( m_commitPage )
        {
            m_commitPage = false;
            m_pages.clear();
        }

        if ( currentPage == ui->loginPage )
            ui->stackedWidget->setCurrentWidget( ui->accessPage );
        else if ( currentPage == ui->accessPage )
#ifdef Q_OS_WIN32
            ui->stackedWidget->setCurrentWidget( ui->pluginsPage );
        else if ( currentPage == ui->pluginsPage )
            ui->stackedWidget->setCurrentWidget( ui->pluginsInstallPage );
        else if ( currentPage == ui->pluginsInstallPage )
            if( aApp->currentSession().user().canBootstrap() && (m_plugins->bootstrappablePlugins().count() > 0) )
                ui->stackedWidget->setCurrentWidget( ui->bootstrapPage );
            else
                ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
#elif defined Q_OS_MAC
            if( aApp->currentSession().user().canBootstrap() )
                ui->stackedWidget->setCurrentWidget( ui->bootstrapPage );
            else
                ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
#elif defined Q_WS_X11
            ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
#endif
        else if ( currentPage == ui->bootstrapPage )
            ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
        else if ( currentPage == ui->tourScrobblesPage )
            ui->stackedWidget->setCurrentWidget( ui->tourMetadataPage );
        // only show the radio page if you can subscribe to get radio
        else if ( currentPage == ui->tourMetadataPage )
        {
            if ( aApp->currentSession().subscriberRadio() )
                ui->stackedWidget->setCurrentWidget( ui->tourRadioPage );
            else
#ifndef Q_WS_X11 // don't show the sys tray page on linux because there isn't one
                ui->stackedWidget->setCurrentWidget( ui->tourLocationPage );
#else
                ui->stackedWidget->setCurrentWidget( ui->tourFinishPage );
#endif
        }
        else if ( currentPage == ui->tourRadioPage )
#ifndef Q_WS_X11 // don't show the sys tray page on linux because there isn't one
            ui->stackedWidget->setCurrentWidget( ui->tourLocationPage );
        else if ( currentPage == ui->tourLocationPage )
#endif
            ui->stackedWidget->setCurrentWidget( ui->tourFinishPage );

        ui->welcome->hide();

        if ( m_showWelcome )
        {
            m_showWelcome = false;
            ui->welcome->setText( tr( "Thanks <strong>%1</strong>, your account is now connected!" ).arg( aApp->currentSession().user().name() ) );
            ui->welcome->show();
        }

        initializePage( ui->stackedWidget->currentWidget() );
    }

    setUpdatesEnabled( true );
}

void
FirstRunWizard::back()
{
    if ( canGoBack() )
    {
        setCommitPage( false );

        ui->welcome->hide();

        cleanupPage( ui->stackedWidget->currentWidget() );
        ui->stackedWidget->setCurrentWidget( m_pages.takeLast() );
        initializePage( ui->stackedWidget->currentWidget() );
    }
}

void
FirstRunWizard::skip()
{
    // skip is mostly the same as next but we don't call validatePage()
    // and the tour pages all go ot the last page

    QWidget* currentPage = ui->stackedWidget->currentWidget();

    // go to the next page
    cleanupPage( currentPage );

    // remember what the last page was so we can go back()
    m_pages << currentPage;

    if ( currentPage == ui->loginPage )
        ui->stackedWidget->setCurrentWidget( ui->accessPage );
    else if ( currentPage == ui->accessPage )
#ifdef Q_OS_WIN
        ui->stackedWidget->setCurrentWidget( ui->pluginsPage );
    else if ( currentPage == ui->pluginsPage )
        ui->stackedWidget->setCurrentWidget( ui->pluginsInstallPage );
    else if ( currentPage == ui->pluginsInstallPage )
        if( aApp->currentSession().user().canBootstrap() && (m_plugins->bootstrappablePlugins().count() > 0) )
            ui->stackedWidget->setCurrentWidget( ui->bootstrapPage );
        else
            ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
#elif defined Q_OS_MAC
        if( aApp->currentSession().user().canBootstrap() )
            ui->stackedWidget->setCurrentWidget( ui->bootstrapPage );
        else
            ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
#elif defined Q_WS_X11
        ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
#endif
    else if ( currentPage == ui->bootstrapPage )
        ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
    else if ( currentPage == ui->bootstrapProgressPage )
        ui->stackedWidget->setCurrentWidget( ui->tourScrobblesPage );
    else if ( currentPage == ui->tourScrobblesPage
              || currentPage == ui->tourMetadataPage
              || currentPage == ui->tourRadioPage
              || currentPage == ui->tourLocationPage )
        ui->stackedWidget->setCurrentWidget( ui->tourFinishPage );

    ui->welcome->hide();

    initializePage( ui->stackedWidget->currentWidget() );
}

void
FirstRunWizard::cleanupPage( QWidget* widget )
{
    WizardPage* page = qobject_cast<WizardPage*>(widget);

    // disconect any buttons from page
    disconnect( ui->next, 0, page, 0 );
    disconnect( ui->back, 0, page, 0 );
    disconnect( ui->finish, 0, page, 0 );
    disconnect( ui->custom, 0, page, 0 );
    disconnect( ui->skip, 0, page, 0 );

    page->cleanupPage();
}

void
FirstRunWizard::initializePage( QWidget* widget )
{
    WizardPage* page = qobject_cast<WizardPage*>(widget);

    // hide all the buttons
    ui->next->hide();
    ui->back->hide();
    ui->finish->hide();
    ui->custom->hide();
    ui->skip->hide();

    page->initializePage();
}


void
FirstRunWizard::onBootstrapStarted( const QString& pluginId )
{
    m_bootstrapState = BootstrapStarted;

    ui->bootstrapProgressPage->setPluginId( pluginId );

    ui->importLabel->setText( tr( "Importing..." ) );

    QMovie* movie = new QMovie( ":/graphic_import.gif", "GIF", this );
    movie->setCacheMode( QMovie::CacheAll );
    ui->importIcon->setMovie( movie );
    movie->start();

    ui->importIcon->show();
    ui->importLabel->show();
}

void
FirstRunWizard::onBootstrapDone( AbstractBootstrapper::BootstrapStatus status )
{
    m_bootstrapState = BootstrapFinished;
    m_bootstrapStatus = status;

    ui->importIcon->setPixmap( QPixmap( ":/lastfm_icon_32.png" ) );
    ui->importLabel->setText( tr( "Import complete!" ) );
}

void
FirstRunWizard::onWizardCompleted()
{
    unicorn::Settings().setFirstRunWizardCompleted( true );
}


void
FirstRunWizard::onRejected()
{
}
