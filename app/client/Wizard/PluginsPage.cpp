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

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QProcess>
#include <QDebug>
#include <QStyle>

#ifdef Q_OS_WIN32
#include "lib/unicorn/plugins/PluginList.h"
#endif

#include "FirstRunWizard.h"
#include "PluginsPage.h"

PluginsPage::PluginsPage()
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 20 );

    // add the radio buttons
    m_pluginsLayout = new QVBoxLayout();
    m_pluginsLayout->setContentsMargins( 0, 0, 0, 0 );
    m_pluginsLayout->setSpacing( 6 );

    layout->addLayout( m_pluginsLayout );

    layout->addWidget( ui.description = new QLabel( tr( "<p>Your media players need a special Last.fm plugin to be able to scrobble the music you listen to.</p>"
                                                       "<p>Please select the media players that you would like to scrobble your music from and click <strong>Install Plugins</strong></p>"), this ),
                                                       0,
                                                       Qt::AlignTop );
    ui.description->setObjectName( "description" );
    ui.description->setWordWrap( true );
}


bool
PluginsPage::validatePage()
{
    return true;
}

void
PluginsPage::cleanupPage()
{
}

void
PluginsPage::initializePage()
{
#ifdef Q_OS_WIN32
    QList<unicorn::IPluginInfo*> supportedPlugins = wizard()->pluginList()->supportedList();
    foreach( unicorn::IPluginInfo* plugin, supportedPlugins )
    {
        QCheckBox* cb;
        m_pluginsLayout->addWidget( cb = new QCheckBox( plugin->name(), this ));
        connect( cb, SIGNAL(toggled(bool)), plugin, SLOT(install(bool)));
        connect( cb, SIGNAL(toggled(bool)), SLOT(checkPluginsSelected()));

        cb->setObjectName( plugin->id() );
        cb->setChecked( plugin->isAppInstalled() );
        cb->style()->polish( cb );

        if ( plugin->isInstalled() )
        {
            if ( plugin->version() > plugin->installedVersion() )
            {
                cb->setChecked( true );
                cb->setText( cb->text() + " " + tr( "(newer version)" ));
            }
            else
            {
                cb->setChecked( false );
                cb->setText( cb->text() + " " + tr( "(Plugin installed tick to reinstall)" ));
            }
        }
    }


    m_pluginsLayout->addStretch();

    setTitle( tr( "Next step, install the Last.fm plugins to be able to scrobble the music you listen to." ));

    checkPluginsSelected();
    if ( wizard()->canGoBack() )
        wizard()->setButton( FirstRunWizard::BackButton, tr( "<< Back" ) );
#endif
}

void
PluginsPage::checkPluginsSelected()
{
#ifdef Q_OS_WIN32
    wizard()->setButton( FirstRunWizard::SkipButton, tr( "Skip >>" ) )->setVisible( wizard()->pluginList()->installList().count() > 0 );
    wizard()->setButton( FirstRunWizard::NextButton,
                         wizard()->pluginList()->installList().count() > 0 ?
                             tr( "Install Plugins" ):
                             tr( "Continue" ) );
#endif
}
