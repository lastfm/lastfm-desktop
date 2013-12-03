TARGET = unicorn
TEMPLATE = lib
CONFIG += dll lastfm sparkle growl logger
unix:!mac {
    CONFIG -= dll
    CONFIG += staticlib
    QMAKE_DISTCLEAN += -f ../../_bin/libunicorn.a
}

QT = core gui xml network
include( ../../admin/include.qmake )
DEFINES += _UNICORN_DLLEXPORT LASTFM_COLLAPSE_NAMESPACE

DEFINES += API_KEY=\\\"$(LASTFM_API_KEY)\\\"
DEFINES += API_SECRET=\\\"$(LASTFM_API_SECRET)\\\"

# UniqueApplication
win32:LIBS += user32.lib shell32.lib ole32.lib kernel32.lib psapi.lib

macx:LIBS += -weak_framework Cocoa

SOURCES += \
    dialogs/ShareDialog.cpp \
    widgets/AvatarWidget.cpp \
    layouts/FlowLayout.cpp \
    widgets/UserMenu.cpp \
    widgets/SlidingStackedWidget.cpp \
    widgets/UserToolButton.cpp \
    widgets/UserManagerWidget.cpp \
    widgets/UnicornTabWidget.cpp \
    widgets/TrackWidget.cpp \
    widgets/TagListWidget.cpp \
    widgets/StatusLight.cpp \
    widgets/SearchBox.cpp \
    widgets/MessageBar.cpp \
    widgets/LfmListViewWidget.cpp \
    widgets/Label.cpp \
    widgets/ItemSelectorWidget.cpp \
    widgets/ImageButton.cpp \
    widgets/HttpImageWidget.cpp \
    widgets/GhostWidget.cpp \
    widgets/FriendsPicker.cpp \
    widgets/DataListWidget.cpp \
    widgets/BannerWidget.cpp \
    widgets/ActionButton.cpp \
    UpdateInfoFetcher.cpp \
    UnicornSettings.cpp \
    UnicornSession.cpp \
    UnicornMainWindow.cpp \
    UnicornCoreApplication.cpp \
    UnicornApplication.cpp \
    TrackImageFetcher.cpp \
    ScrobblesModel.cpp \
    qtwin.cpp \
    qtsingleapplication/qtsinglecoreapplication.cpp \
    qtsingleapplication/qtsingleapplication.cpp \
    qtsingleapplication/qtlockedfile_unix.cpp \
    qtsingleapplication/qtlockedfile.cpp \
    qtsingleapplication/qtlocalpeer.cpp \
    QMessageBoxBuilder.cpp \
    LoginProcess.cpp \
    PlayBus/PlayBus.cpp \
    PlayBus/Bus.cpp \
    dialogs/UserManagerDialog.cpp \
    dialogs/TagDialog.cpp \
    dialogs/LoginDialog.cpp \
    dialogs/LoginContinueDialog.cpp \
    dialogs/AboutDialog.cpp \
    dialogs/ScrobbleConfirmationDialog.cpp \
    dialogs/CloseAppsDialog.cpp \
    AnimatedStatusBar.cpp \
    DesktopServices.cpp \
    Updater/Updater.cpp \
    widgets/StackedWidget.cpp \
    widgets/ProxyWidget.cpp \
    dialogs/ProxyDialog.cpp \
    plugins/Version.cpp

HEADERS += \
    widgets/UserToolButton.h \
    widgets/UserMenu.h \
    widgets/UserManagerWidget.h \
    widgets/UserComboSelector.h \
    widgets/UnicornTabWidget.h \
    widgets/TrackWidget.h \
    widgets/TagListWidget.h \
    widgets/StatusLight.h \
    widgets/SpinnerLabel.h \
    widgets/Seed.h \
    widgets/SearchBox.h \
    widgets/PlayableMimeData.h \
    widgets/MessageBar.h \
    widgets/LfmListViewWidget.h \
    widgets/Label.h \
    widgets/ItemSelectorWidget.h \
    widgets/ImageButton.h \
    widgets/HttpImageWidget.h \
    widgets/GhostWidget.h \
    widgets/FriendsPicker.h \
    widgets/DataListWidget.h \
    widgets/DataBox.h \
    widgets/BannerWidget.h \
    widgets/ActionButton.h \
    UpdateInfoFetcher.h \
    UnicornSettings.h \
    UnicornSession.h \
    UnicornMainWindow.h \
    UnicornCoreApplication.h \
    UnicornApplication.h \
    TrackImageFetcher.h \
    SignalBlocker.h \
    ScrobblesModel.h \
    qtwin.h \
    qtsingleapplication/qtsinglecoreapplication.h \
    qtsingleapplication/qtsingleapplication.h \
    qtsingleapplication/qtlockedfile.h \
    qtsingleapplication/qtlocalpeer.h \
    QMessageBoxBuilder.h \
    PlayBus/Bus.h \
    PlayBus/PlayBus.h \
    LoginProcess.h \
    dialogs/UserManagerDialog.h \
    dialogs/UnicornDialog.h \
    dialogs/TagDialog.h \
    dialogs/LoginDialog.h \
    dialogs/LoginContinueDialog.h \
    dialogs/AboutDialog.h \
    dialogs/ScrobbleConfirmationDialog.h \
    dialogs/CloseAppsDialog.h \
    AnimatedStatusBar.h \
    AnimatedPushButton.h \
    dialogs/ShareDialog.h \
    widgets/AvatarWidget.h \
    layouts/FlowLayout.h \
    widgets/SlidingStackedWidget.h \
    Updater/Updater.h \
    DesktopServices.h \
    widgets/StackedWidget.h \
    widgets/ProxyWidget.h \
    dialogs/ProxyDialog.h \
    plugins/Version.h
	
win32:HEADERS += plugins/FooBar08PluginInfo.h \
                    plugins/FooBar09PluginInfo.h \
                    plugins/ITunesPluginInfo.h \
                    plugins/WinampPluginInfo.h \
                    plugins/WmpPluginInfo.h \
                    plugins/PluginList.h \
                    plugins/KillProcess.h \
                    plugins/IPluginInfo.h

win32:SOURCES += qtsingleapplication/qtlockedfile_win.cpp\
                    plugins/PluginList.cpp \
                    plugins/IPluginInfo.cpp \
                    plugins/FooBar08PluginInfo.cpp \
                    plugins/FooBar09PluginInfo.cpp \
                    plugins/ITunesPluginInfo.cpp \
                    plugins/WinampPluginInfo.cpp \
                    plugins/WmpPluginInfo.cpp

macx:SOURCES += mac/AppleScript.cpp \
                plugins/ITunesPluginInstaller.cpp

macx:OBJECTIVE_SOURCES += UnicornApplication_mac.mm \
                          notify/Notify.mm \
                          Updater/Updater_mac.mm \
                          UnicornApplicationDelegate.mm \
                          dialogs/CloseAppsDialog_mac.mm

macx:HEADERS += mac/AppleScript.h \
                notify/Notify.h \
                UnicornApplicationDelegate.h \
                plugins/ITunesPluginInstaller.h \

CONFIG( break ) {
    HEADERS += CrashReporter/CrashReporter.h
    SOURCES += CrashReporter/CrashReporter.cpp
    macx:OBJECTIVE_SOURCES += CrashReporter/CrashReporter_mac.mm
}
                          
FORMS += \
	dialogs/ShareDialog.ui \
    dialogs/TagDialog.ui \
    dialogs/AboutDialog.ui \
    dialogs/ScrobbleConfirmationDialog.ui \
    widgets/ProxyWidget.ui \
    dialogs/ProxyDialog.ui \
    dialogs/CloseAppsDialog.ui

unix:!mac {
    FORMS   -= dialogs/CloseAppsDialog.ui

    SOURCES -= dialogs/CloseAppsDialog.cpp \
               plugins/Version.cpp \
               Updater/Updater.cpp

    HEADERS -= dialogs/CloseAppsDialog.cpp \
               plugins/Version.h \
               Updater/Updater.h
}
