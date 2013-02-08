#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QPointer>

#include "lib/DllExportMacro.h"

#ifdef Q_OS_WIN
namespace qtsparkle { class Updater; }
#define UPDATE_URL_WIN "http://cdn.last.fm/client/updates/updates.win.xml"
#define UPDATE_URL_WIN_BETA "http://cdn.last.fm/client/updates/updates.win.beta.xml"
#elif defined( Q_OS_MAC )
#define UPDATE_URL_MAC @"http://cdn.last.fm/client/updates/updates.mac.xml"
#define UPDATE_URL_MAC_BETA @"http://cdn.last.fm/client/updates/updates.mac.beta.xml"
#endif

namespace unicorn
{

class UNICORN_DLLEXPORT Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater( QWidget* parent = 0 );
    ~Updater();

    void setBetaUpdates( bool betaUpdates );

public slots:
    void checkForUpdates();

private:
#ifdef Q_OS_WIN
    QPointer<qtsparkle::Updater> m_updater;
    QWidget* m_parentWidget;
    bool m_betaUpdates;
#endif
};

}

#endif // UPDATER_H
