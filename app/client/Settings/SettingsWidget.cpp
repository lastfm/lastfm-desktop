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

#include "SettingsWidget.h"

SettingsWidget::SettingsWidget( QWidget* parent )
    : QWidget( parent )
    , m_settingsChanged( false )
{
}

void
SettingsWidget::onSettingsChanged()
{
    m_settingsChanged = true;
#ifdef Q_OS_MAC
    saveSettings();
#else
    emit settingsChanged();
#endif
}

void
SettingsWidget::onSettingsSaved()
{
    m_settingsChanged = false;
}

bool
SettingsWidget::hasUnsavedChanges()
{
    return m_settingsChanged;
}
