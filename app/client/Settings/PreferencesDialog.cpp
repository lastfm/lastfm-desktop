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

#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QShortcut>

#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"

PreferencesDialog::PreferencesDialog( QMenuBar* menuBar, QWidget* parent )
    :unicorn::MainWindow( menuBar, parent ),
    ui( new Ui::PreferencesDialog )
{
    // Disable the minimize and maximize buttons.
    Qt::WindowFlags flags = this->windowFlags();
    flags &= ~Qt::WindowMinMaxButtonsHint;
    setWindowFlags(flags);

    ui->setupUi( this );

    setAttribute( Qt::WA_DeleteOnClose, true );

    static_cast<QAbstractButton*>( ui->toolBar->widgetForAction( ui->actionGeneral ) )->setAutoExclusive( true );
    static_cast<QAbstractButton*>( ui->toolBar->widgetForAction( ui->actionScrobbling ) )->setAutoExclusive( true );
    static_cast<QAbstractButton*>( ui->toolBar->widgetForAction( ui->actionDevices ) )->setAutoExclusive( true );
    static_cast<QAbstractButton*>( ui->toolBar->widgetForAction( ui->actionAccounts ) )->setAutoExclusive( true );
    static_cast<QAbstractButton*>( ui->toolBar->widgetForAction( ui->actionAdvanced ) )->setAutoExclusive( true );

    connect( ui->toolBar->widgetForAction( ui->actionGeneral ), SIGNAL(toggled(bool)), ui->actionGeneral, SLOT(setChecked(bool)) );
    connect( ui->toolBar->widgetForAction( ui->actionScrobbling ), SIGNAL(toggled(bool)), ui->actionScrobbling, SLOT(setChecked(bool)) );
    connect( ui->toolBar->widgetForAction( ui->actionDevices ), SIGNAL(toggled(bool)), ui->actionDevices, SLOT(setChecked(bool)) );
    connect( ui->toolBar->widgetForAction( ui->actionAccounts ), SIGNAL(toggled(bool)), ui->actionAccounts, SLOT(setChecked(bool)) );
    connect( ui->toolBar->widgetForAction( ui->actionAdvanced ), SIGNAL(toggled(bool)), ui->actionAdvanced, SLOT(setChecked(bool)) );

    connect( this, SIGNAL( saveNeeded() ), ui->general, SLOT( saveSettings() ) );
    connect( this, SIGNAL( saveNeeded() ), ui->scrobbling, SLOT( saveSettings() ) );
    connect( this, SIGNAL( saveNeeded() ), ui->ipod, SLOT( saveSettings() ) );
    connect( this, SIGNAL( saveNeeded() ), ui->accounts, SLOT( saveSettings() ) );
    connect( this, SIGNAL( saveNeeded() ), ui->advanced, SLOT( saveSettings() ) );

    connect( ui->general, SIGNAL( settingsChanged() ), SLOT( onSettingsChanged() ) );
    connect( ui->scrobbling, SIGNAL( settingsChanged() ), SLOT( onSettingsChanged() ) );
    connect( ui->ipod, SIGNAL( settingsChanged() ), SLOT( onSettingsChanged() ) );
    connect( ui->accounts, SIGNAL( settingsChanged() ), SLOT( onSettingsChanged() ) );
    connect( ui->advanced, SIGNAL( settingsChanged() ), SLOT( onSettingsChanged() ) );

    connect( ui->actionGeneral, SIGNAL(triggered()), SLOT(onTabButtonClicked()));
    connect( ui->actionScrobbling, SIGNAL(triggered()), SLOT(onTabButtonClicked()));
    connect( ui->actionDevices, SIGNAL(triggered()), SLOT(onTabButtonClicked()));
    connect( ui->actionAccounts, SIGNAL(triggered()), SLOT(onTabButtonClicked()));
    connect( ui->actionAdvanced, SIGNAL(triggered()), SLOT(onTabButtonClicked()));

    connect( ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(onStackCurrentChanged(int)), Qt::QueuedConnection );

#ifdef Q_OS_MAC
    ui->buttonBox->hide();
#endif
    connect( ui->buttonBox, SIGNAL( accepted() ), SLOT( onAccepted() ) );
    connect( ui->buttonBox, SIGNAL( rejected() ), SLOT( onRejected() ) );

    QAbstractButton* applyButton = ui->buttonBox->button( QDialogButtonBox::Apply );
    applyButton->setEnabled( false );
    connect( applyButton, SIGNAL( clicked() ), SLOT( onApplyButtonClicked() ) );

    setFixedWidth( 550 );

    ui->stackedWidget->setCurrentWidget( ui->accounts );
    ui->actionGeneral->trigger();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void
PreferencesDialog::onTabButtonClicked()
{
    QAction* clickedButton = qobject_cast<QAction*>(sender());

    setWindowTitle( clickedButton->text() );

    if ( clickedButton == ui->actionGeneral )
        ui->stackedWidget->setCurrentWidget( ui->general );
    else if ( clickedButton == ui->actionAccounts )
        ui->stackedWidget->setCurrentWidget( ui->accounts );
    else if ( clickedButton == ui->actionDevices )
        ui->stackedWidget->setCurrentWidget( ui->ipod );
    else if ( clickedButton == ui->actionAdvanced )
        ui->stackedWidget->setCurrentWidget( ui->advanced );
    else if ( clickedButton == ui->actionScrobbling )
        ui->stackedWidget->setCurrentWidget( ui->scrobbling );
}

void
PreferencesDialog::onStackCurrentChanged( int /*index*/ )
{
    adjustSize();
}

void
PreferencesDialog::onAccepted()
{
    emit saveNeeded();
    close();
}

void
PreferencesDialog::onRejected()
{
    close();
}

void
PreferencesDialog::onSettingsChanged()
{
    ui->buttonBox->button( QDialogButtonBox::Apply )->setEnabled( true );
}

void
PreferencesDialog::onApplyButtonClicked()
{
    emit saveNeeded();
    ui->buttonBox->button( QDialogButtonBox::Apply )->setEnabled( false );
}
