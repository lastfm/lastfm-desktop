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

#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

#include "lib/unicorn/QMessageBoxBuilder.h"

#include "lib/unicorn/dialogs/CloseAppsDialog.h"

#include "FirstRunWizard.h"
#include "PluginsInstallPage.h"

PluginsInstallPage::PluginsInstallPage()
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 20 );

    layout->addWidget( ui.image = new QLabel( this ), 0, Qt::AlignTop );
    ui.image->setObjectName( "image" );
    layout->addWidget( ui.description = new QLabel( tr( "<p>Please follow the instructions that appear from your operating system to install the plugins.</p>"
                                                          "<p>Once the plugins have been installed on you computer, click <strong>Continue</strong>.</p>"), this ),
                         0,
                         Qt::AlignTop);
    ui.description->setObjectName( "description" );
    ui.description->setWordWrap( true );
}


void
PluginsInstallPage::initializePage()
{
    setTitle( tr( "Your plugins are now being installed" ) );

    wizard()->setCommitPage( true );

#ifdef Q_OS_WIN32
    QAbstractButton* continueButton = wizard()->setButton( FirstRunWizard::NextButton, tr( "Continue" ) );
    if ( wizard()->canGoBack() )
        wizard()->setButton( FirstRunWizard::BackButton, tr( "<< Back" ) );


    if ( wizard()->pluginList()->installList().count() > 0 )
    {
        // get the install to happen a bit later so that
        // we actually show this page of the wizard
        // and the user has time to read what's happening
        QTimer::singleShot( 1000, this, SLOT(install()) );
        continueButton->setEnabled( false );
    }
    else
    {
        continueButton->click();
    }
#endif
}

void
PluginsInstallPage::install()
{
#ifdef Q_OS_WIN32
    // Tell the user to close any aplications we are about to install
    // plugins for

    unicorn::CloseAppsDialog* closeApps = new unicorn::CloseAppsDialog( wizard()->pluginList()->installList(), this );

    if ( closeApps->result() != QDialog::Accepted )
        closeApps->exec();
    else
        closeApps->deleteLater();

    if ( closeApps->result() == QDialog::Accepted )
    {
        foreach( unicorn::IPluginInfo* plugin, wizard()->pluginList()->installList() )
        {
            if ( !plugin->doInstall() )
            {
                // The plugin failed to install
                // should probably do something
            }
        }
    }
    else
    {
        // The user didn't close their media players
        QMessageBoxBuilder( this ).setTitle( tr( "Your plugins haven't been installed" ) )
                .setIcon( QMessageBox::Warning )
                .setText( tr( "You can install them later through the file menu" ) )
                .setButtons( QMessageBox::Ok )
                .exec();
    }
#endif

    QAbstractButton* continueButton = wizard()->setButton( FirstRunWizard::NextButton, tr( "Continue" ) );
    continueButton->setEnabled( true );
    QTimer::singleShot( 1000, continueButton, SLOT(click()) );
}


void
PluginsInstallPage::cleanupPage()
{
}
