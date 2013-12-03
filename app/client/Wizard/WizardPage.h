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

#ifndef WIZARDPAGE_H
#define WIZARDPAGE_H

#include <QWidget>

class FirstRunWizard;

class WizardPage : public QWidget
{
    Q_OBJECT
public:
    explicit WizardPage();

    void setWizard( FirstRunWizard* wizard );

    virtual void initializePage() = 0;
    virtual bool validatePage();
    virtual void cleanupPage() = 0;

    void setTitle( const QString& title );

protected:
    FirstRunWizard* wizard() const;

private:
    FirstRunWizard* m_wizard;
};

#endif // WIZARDPAGE_H
