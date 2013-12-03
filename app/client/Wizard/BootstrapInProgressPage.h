/*
   Copyright 2010-2011 Last.fm Ltd.
      - Primarily authored by Jono Cole and Michael Coffey

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

#ifndef BOOTSTRAP_IN_PROGRESS_PAGE_H
#define BOOTSTRAP_IN_PROGRESS_PAGE_H

#include <QWizardPage>
#include <types/Track.h>

#ifdef Q_OS_WIN
#include "lib/unicorn/Updater/PluginList.h"
#endif

class BootstrapInProgressPage : public QWizardPage {
    Q_OBJECT
public:
    BootstrapInProgressPage( QWizard* p );

    void initializePage();

    virtual bool isComplete() const;

protected slots:
    void startBootstrap();
    void onTrackProcessed( int percentage, const Track& );
    void onPercentageUpload( int percentage );
    void onBootstrapDone( int );
    void onTrackStarted( const Track& );

protected:
    class QProgressBar* m_progressBar;
    class QLabel* m_label;

    bool m_isComplete;
    class AbstractBootstrapper* m_bootstrapper;
#ifdef Q_OS_WIN
    PluginList m_pluginList;
#endif
};

#endif //BOOTSTRAP_IN_PROGRESS_PAGE_H

