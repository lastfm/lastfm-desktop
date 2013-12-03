/*
   Copyright 2005-2009 Last.fm Ltd.
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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

#ifndef METADATA_WINDOW_H_
#define METADATA_WINDOW_H_

#include <QResizeEvent>

#include <lastfm/Track.h>
#include <lastfm/XmlQuery.h>

#include "lib/unicorn/UnicornMainWindow.h"

#ifdef  Q_OS_MAC
#include "lib/unicorn/plugins/ITunesPluginInstaller.h"
#endif

#include "Settings/PreferencesDialog.h"
#include "Dialogs/DiagnosticsDialog.h"

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
namespace unicorn
{
    class Updater;
    class PluginList;
}
using unicorn::Updater;
#endif

using lastfm::XmlQuery;

class QAbstractButton;
class QTabBar;
class QLabel;
class ScrobbleControls;
class FirstRunWizard;
class MessageBar;
class SlideOverLayout;
class UserToolButton;

class IpodDevice;

class MainWindow : public unicorn::MainWindow
{
    Q_OBJECT

    struct{
        class TitleBar* titleBar;
        class StatusBar* statusBar;

        class MessageBar* messageBar;

        class SideBar* sideBar;
        class QStackedWidget* stackedWidget;

        class NowPlayingStackedWidget* nowPlaying;
        class ScrobblesWidget* scrobbles;

        class QScrollArea* profileScrollArea;
        QWidget* profile;
        class QScrollArea* friendsScrollArea;
        QWidget* friends;
        class QScrollArea* radioScrollArea;
        class RadioWidget* radio;

    } ui;

public:
    MainWindow( QMenuBar* );
    const Track& currentTrack() const{ return m_currentTrack; }

    void addWinThumbBarButton( QAction* );
    void setBetaUpdates( bool betaUpdates );

    QString currentCategory() const;

signals:
    void trackGotInfo(XmlQuery);
    void albumGotInfo(XmlQuery);
    void artistGotInfo(XmlQuery);
    void artistGotEvents(XmlQuery);
    void trackGotTopFans(XmlQuery);
    void trackGotTags(XmlQuery);
    void finished();

public slots:
    void onPrefsTriggered();
    void onDiagnosticsTriggered();
    void onMinimizeTriggered();
    void onZoomTriggered();
    void onBringAllToFrontTriggered();
    void showMessage( const QString& message, const QString& id = "", int timeout = -1 /*seconds*/ );

private slots:
    void onVisitProfile();

    void onTrackStarted(const lastfm::Track&, const lastfm::Track&);
    void onStopped();
    void onPaused();
    void onResumed();
    void onTuningIn();

    void onRadioError( int error, const QVariant& data );
    void onRadioMessage(const QString& message);

    void checkForUpdates();

    void onSpace();

    void onConfigRetrieved();

#ifdef Q_OS_WIN32
    void checkUpdatedPlugins();
#endif

    // iPod scrobbling things
    void onFoundScrobbles( const QList<lastfm::Track>& tracks );

private:
    void setCurrentWidget( QWidget* );
    void addWinThumbBarButtons( QList<QAction*>& );
    void setupMenuBar();

    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

    static QString applicationName();

private:
    Track m_currentTrack;
    class ActivityListItem* m_currentActivity;
    class QDockWidget* m_radioSideBar;
    QList<QAction*> m_buttons;

    QPointer<PreferencesDialog> m_preferences;
    QPointer<DiagnosticsDialog> m_diagnostics;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    QPointer<Updater> m_updater;
#endif

#ifdef Q_OS_WIN
    QPointer<unicorn::PluginList> m_pluginList;
#endif

#ifdef Q_WS_MAC
    QPointer<unicorn::ITunesPluginInstaller> m_installer;
#endif
};

#endif //METADATA_WINDOW_H_

