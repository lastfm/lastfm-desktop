
#include "Updater.h"

#ifndef Q_OS_MAC

#ifdef Q_OS_WIN
#include <QCoreApplication>
#include <QStringList>
#include <qtsparkle/Updater>
#endif

#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/UnicornApplication.h"

unicorn::Updater::Updater( QWidget* parent ) :
    QObject(parent), m_parentWidget( parent )
{
#ifdef Q_OS_WIN

    if ( qApp->arguments().contains( "--update" ) )
    {
        int urlIndex = qApp->arguments().indexOf( "--update" ) + 1;

        if ( qApp->arguments().count() > urlIndex && qApp->arguments()[urlIndex].startsWith( "http://" ) )
        {
            m_updater = new qtsparkle::Updater( QUrl( QString( qApp->arguments()[urlIndex].toUtf8() ) ), m_parentWidget );
            m_updater->SetNetworkAccessManager( lastfm::nam() );
            m_updater->SetIcon( QPixmap( ":/scrobbler_64.png" ) );
        }
        else
            setBetaUpdates( unicorn::Settings().value( SETTING_BETA_UPGRADES, false ).toBool() );
    }
    else
        setBetaUpdates( unicorn::Settings().value( SETTING_BETA_UPGRADES, false ).toBool() );


#endif
}

void
unicorn::Updater::setBetaUpdates( bool betaUpdates )
{
#ifdef Q_OS_WIN
    if ( !m_updater || m_betaUpdates != betaUpdates )
    {
        // there is no updater yet or we are changing the update file
        m_betaUpdates = betaUpdates;

        delete m_updater;
        m_updater = new qtsparkle::Updater( QUrl( QString( betaUpdates ? UPDATE_URL_WIN_BETA : UPDATE_URL_WIN ) ), m_parentWidget );
        m_updater->SetNetworkAccessManager( lastfm::nam() );
        m_updater->SetIcon( QPixmap( ":/scrobbler_64.png" ) );
    }
#endif
}

void
unicorn::Updater::checkForUpdates()
{
#ifdef Q_OS_WIN
    m_updater->CheckNow();
#endif
}

unicorn::Updater::~Updater()
{
#ifdef Q_OS_WIN
    delete m_updater;
#endif
}

#endif // Q_OS_MAC
