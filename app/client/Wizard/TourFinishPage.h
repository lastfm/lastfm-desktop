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

#ifndef TOURFINISHPAGE_H
#define TOURFINISHPAGE_H

#include "WizardPage.h"
#include "../Bootstrapper/AbstractBootstrapper.h"
#include "FirstRunWizard.h"

class TourFinishPage : public WizardPage
{
    Q_OBJECT
private:
    struct
    {
        class QLabel* image;
        class QLabel* description;
    } ui;

public:
    explicit TourFinishPage();

private slots:
    void onBootstrapDone( AbstractBootstrapper::BootstrapStatus status );

private:
    void setDescription( FirstRunWizard::BootstrapState state, AbstractBootstrapper::BootstrapStatus status );
    void initializePage();
    void cleanupPage();
};

#endif // TOURFINISHPAGE_H
