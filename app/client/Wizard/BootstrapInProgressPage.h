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

