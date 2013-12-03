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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

#include "lib/unicorn/dialogs/UnicornDialog.h"

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public unicorn::MainWindow
{
    Q_OBJECT

public:
    explicit PreferencesDialog( QMenuBar* menuBar, QWidget* parent = 0 );
    ~PreferencesDialog();

signals:
    void saveNeeded();

private slots:
    void onTabButtonClicked();

    void onAccepted();
    void onRejected();

    void onSettingsChanged();
    void onApplyButtonClicked();

    void onStackCurrentChanged( int index );

private:
    Ui::PreferencesDialog *ui;
};

#endif // PREFERENCESDIALOG_H
