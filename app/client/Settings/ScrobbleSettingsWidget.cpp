/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Jono Cole, Michael Coffey, and William Viana

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

#include <QCheckBox>
#include <QDebug>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

#include "lib/unicorn/UnicornSettings.h"

#include "../Application.h"
#include "../Services/ScrobbleService/ScrobbleService.h"
#include "../Services/AnalyticsService.h"

#include "ui_ScrobbleSettingsWidget.h"
#include "ScrobbleSettingsWidget.h"

ScrobbleSettingsWidget::ScrobbleSettingsWidget( QWidget* parent )
    : SettingsWidget( parent ),
      ui( new Ui::ScrobbleSettingsWidget )
{
    ui->setupUi( this );

    unicorn::UserSettings userSettings;

    double scrobblePointValue = userSettings.scrobblePoint();
    ui->scrobblePoint->setValue( scrobblePointValue );
    ui->percentText->setText( QString::number(scrobblePointValue) );
    ui->percentText->setFixedWidth( ui->percentText->fontMetrics().width( "100" ) );
    m_initialScrobblePercentage = scrobblePointValue;

    ui->allowFingerprint->setChecked( userSettings.fingerprinting() );

    ui->enfocreScrobbleTimeMax->setChecked( userSettings.enforceScrobbleTimeMax() );
    ui->scrobblingOn->setChecked( userSettings.scrobblingOn() );
    ui->podcasts->setChecked( userSettings.podcasts() );

    QStringList exclusionDirs = userSettings.exclusionDirs();
    exclusionDirs.removeAll( "" );
    ui->exclusionDirs->setExclusions( exclusionDirs );

    connect( ui->scrobblePoint, SIGNAL(sliderMoved(int)), SLOT(onSliderMoved(int)) );
    connect( ui->scrobblePoint, SIGNAL(valueChanged(int)), SLOT(onSliderMoved(int)) );
    connect( ui->scrobblePoint, SIGNAL(valueChanged(int)), SLOT(onSettingsChanged()) );
    connect( ui->allowFingerprint, SIGNAL(stateChanged(int)), SLOT(onSettingsChanged()) );

    connect( aApp, SIGNAL(scrobbleToggled(bool)), ui->scrobblingOn, SLOT(setChecked(bool)));
    connect( ui->scrobblingOn, SIGNAL(clicked(bool)), SLOT(onSettingsChanged()) );
    connect( ui->podcasts, SIGNAL(stateChanged(int)), SLOT(onSettingsChanged()) );
    connect( ui->enfocreScrobbleTimeMax, SIGNAL(stateChanged(int)), SLOT(onSettingsChanged()) );

    connect( ui->exclusionDirs, SIGNAL(dataChanged()), SLOT(onSettingsChanged()) );
}

ScrobbleSettingsWidget::~ScrobbleSettingsWidget()
{
    if ( unicorn::UserSettings().scrobblePoint() != m_initialScrobblePercentage )
        AnalyticsService::instance().sendEvent(SETTINGS_CATEGORY, SCROBBLING_SETTINGS, "ScrobblePercentageChanged", QString::number( ui->scrobblePoint->value() ) );
}

void
ScrobbleSettingsWidget::onSliderMoved( int value )
{
    ui->percentText->setText( QString::number( value ) );
}

void
ScrobbleSettingsWidget::saveSettings()
{
    if ( hasUnsavedChanges() )
    {
        qDebug() << "Saving settings...";

        aApp->onScrobbleToggled( ui->scrobblingOn->isChecked() );

        unicorn::UserSettings userSettings;
        userSettings.setScrobblePoint( ui->scrobblePoint->value() );
        userSettings.setFingerprinting( ui->allowFingerprint->isChecked() );
        userSettings.setPodcasts( ui->podcasts->isChecked() );
        userSettings.setEnforceScrobbleTimeMax( ui->enfocreScrobbleTimeMax->isChecked() );

        QStringList exclusionDirs = ui->exclusionDirs->getExclusions();
        exclusionDirs.removeAll( "" );
        qDebug() << exclusionDirs;
        userSettings.setExclusionDirs( exclusionDirs );

        userSettings.sync();

        ScrobbleService::instance().scrobbleSettingsChanged();

        onSettingsSaved();
    }
}
