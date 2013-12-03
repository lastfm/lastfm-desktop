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

#ifndef TOUR_LOCATION_PAGE_H
#define TOUR_LOCATION_PAGE_H

#include "WizardPage.h"
#include <QIcon>
#include <QPointer>

class PointyArrow;

class TourLocationPage : public WizardPage
{
    Q_OBJECT
private:
    struct
    {
        class QLabel* image;
        class QLabel* description;
    } ui;

public:
    TourLocationPage();
    ~TourLocationPage();

    void initializePage();
    void cleanupPage();

protected slots:
    void flashSysTray();

private:
    QPointer<PointyArrow> m_arrow;
    QPointer<QTimer> m_flashTimer;
    QIcon m_transparentIcon;
    QIcon m_normalIcon;
    bool m_flash;
};

#endif // TOUR_METADATA_PAGE_H

