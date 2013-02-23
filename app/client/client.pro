TEMPLATE = app
TARGET = "Last.fm Scrobbler"
unix:!mac {
    TARGET = lastfm-scrobbler
}
VERSION = 2.1.34
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
QT = core gui xml network sql webkit
CONFIG += lastfm unicorn listener logger phonon fingerprint ffmpeg
win32:LIBS += user32.lib kernel32.lib psapi.lib
DEFINES += LASTFM_COLLAPSE_NAMESPACE

macx:LIBS += -weak_framework Cocoa
win32:release {
        LIBS += -lAdvAPI32
}

include( ../../admin/include.qmake )

DEFINES += LASTFM_COLLAPSE_NAMESPACE
SOURCES -= LegacyTuner.cpp
HEADERS -= LegacyTuner.h

macx:ICON = ./audioscrobbler.icns
!win32:LIBS += -lz
win32:LIBS += shell32.lib User32.lib

RC_FILE = audioscrobbler.rc
unix:!mac {
        CONFIG += link_pkgconfig
        PKGCONFIG += libgpod-1.0
}

SUBDIRS=PrefPane

SOURCES += \
    AudioscrobblerSettings.cpp \
    Application.cpp \
    StationSearch.cpp \
    ScrobSocket.cpp \
    MediaDevices/MediaDevice.cpp \
    MediaDevices/IpodDevice.cpp \
    MediaDevices/DeviceScrobbler.cpp \
    MainWindow.cpp \
    main.cpp \
    Settings/SettingsWidget.cpp \
    Settings/ScrobbleSettingsWidget.cpp \
    Settings/IpodSettingsWidget.cpp \
    Settings/AccountSettingsWidget.cpp \
    Settings/PreferencesDialog.cpp \
    Settings/AdvancedSettingsWidget.cpp \
    Settings/GeneralSettingsWidget.cpp \
    Services/ScrobbleService/StopWatch.cpp \
    Services/ScrobbleService/ScrobbleService.cpp \
    Services/RadioService/RadioService.cpp \
    Services/RadioService/RadioConnection.cpp \
    Dialogs/DiagnosticsDialog.cpp \
    Bootstrapper/PluginBootstrapper.cpp \
    Bootstrapper/ITunesDevice/itunesdevice.cpp \
    Bootstrapper/iTunesBootstrapper.cpp \
    Bootstrapper/AbstractFileBootstrapper.cpp \
    Bootstrapper/AbstractBootstrapper.cpp \
    Widgets/TitleBar.cpp \
    Widgets/StatusBar.cpp \
    Widgets/SideBar.cpp \
    Widgets/RadioWidget.cpp \
    Widgets/NothingPlayingWidget.cpp \
    Widgets/NowPlayingStackedWidget.cpp \
    Widgets/ProfileWidget.cpp \
    Widgets/FriendListWidget.cpp \
    Widgets/FriendWidget.cpp \
    Widgets/BioWidget.cpp \
    Widgets/MetadataWidget.cpp \
    Widgets/TagWidget.cpp \
    Widgets/ShortcutEdit.cpp \
    Widgets/ProfileArtistWidget.cpp \
    Widgets/ScrobbleControls.cpp \
    Widgets/ProgressBar.cpp \
    Widgets/QuickStartWidget.cpp \
    Widgets/PointyArrow.cpp \
    Widgets/PlaybackControlsWidget.cpp \
    Widgets/PlayableItemWidget.cpp \
    Widgets/NowPlayingWidget.cpp \
    Widgets/RefreshButton.cpp \
    Widgets/WidgetTextObject.cpp \
    Wizard/LoginPage.cpp \
    Wizard/BootstrapPage.cpp \
    Wizard/FirstRunWizard.cpp \
    Wizard/AccessPage.cpp \
    Wizard/TourMetadataPage.cpp \
    Wizard/PluginsPage.cpp \
    Wizard/TourRadioPage.cpp \
    Wizard/TourFinishPage.cpp \
    Wizard/PluginsInstallPage.cpp \
    Wizard/BootstrapProgressPage.cpp \
    Wizard/TourScrobblesPage.cpp \
    Wizard/TourLocationPage.cpp \
    Wizard/WizardPage.cpp \
    Widgets/ContextLabel.cpp \
    Widgets/SimilarArtistWidget.cpp \
    Widgets/PushButton.cpp \
    Widgets/TrackWidget.cpp \
    Dialogs/LicensesDialog.cpp \
    Widgets/ScrobblesWidget.cpp \
    Widgets/ScrobblesListWidget.cpp \
    Fingerprinter/Fingerprinter.cpp \
    Services/AnalyticsService/AnalyticsService.cpp \
    Services/AnalyticsService/PersistentCookieJar.cpp \
    Settings/CheckFileSystemModel.cpp \
    Settings/CheckFileSystemView.cpp

HEADERS += \
    ScrobSocket.h \
    AudioscrobblerSettings.h \
    Application.h \
    MainWindow.h \
    StationSearch.h \
    Services/RadioService/RadioConnection.h \
    Services/ScrobbleService.h \
    Services/ScrobbleService/StopWatch.h \
    Services/ScrobbleService/ScrobbleService.h \
    Services/RadioService.h \
    Services/RadioService/RadioService.h \
    MediaDevices/MediaDevice.h \
    MediaDevices/IpodDevice.h \
    MediaDevices/DeviceScrobbler.h \
    Dialogs/DiagnosticsDialog.h \
    Bootstrapper/PluginBootstrapper.h \
    Bootstrapper/ITunesDevice/MediaDeviceInterface.h \
    Bootstrapper/ITunesDevice/ITunesParser.h \
    Bootstrapper/ITunesDevice/itunesdevice.h \
    Bootstrapper/iTunesBootstrapper.h \
    Bootstrapper/AbstractFileBootstrapper.h \
    Bootstrapper/AbstractBootstrapper.h \
    Settings/SettingsWidget.h \
    Settings/ScrobbleSettingsWidget.h \
    Settings/PreferencesDialog.h \
    Settings/AdvancedSettingsWidget.h \
    Settings/GeneralSettingsWidget.h \
    Settings/IpodSettingsWidget.h \
    Settings/AccountSettingsWidget.h \
    Widgets/ShortcutEdit.h \
    Widgets/TitleBar.h \
    Widgets/StatusBar.h \
    Widgets/SideBar.h \
    Widgets/ScrobbleControls.h \
    Widgets/ProgressBar.h \
    Widgets/QuickStartWidget.h \
    Widgets/PointyArrow.h \
    Widgets/PlaybackControlsWidget.h \
    Widgets/PlayableItemWidget.h \
    Widgets/NowPlayingWidget.h \
    Widgets/RadioWidget.h \
    Widgets/NothingPlayingWidget.h \
    Widgets/NowPlayingStackedWidget.h \
    Widgets/ProfileWidget.h \
    Widgets/FriendListWidget.h \
    Widgets/FriendWidget.h \
    Widgets/BioWidget.h \
    Widgets/MetadataWidget.h \
    Widgets/TagWidget.h \
    Widgets/ProfileArtistWidget.h \
    Widgets/RefreshButton.h \
    Widgets/WidgetTextObject.h \
    Wizard/AccessPage.h \
    Wizard/TourMetadataPage.h \
    Wizard/PluginsPage.h \
    Wizard/TourRadioPage.h \
    Wizard/TourFinishPage.h \
    Wizard/PluginsInstallPage.h \
    Wizard/BootstrapProgressPage.h \
    Wizard/TourScrobblesPage.h \
    Wizard/TourLocationPage.h \
    Wizard/LoginPage.h \
    Wizard/FirstRunWizard.h \
    Wizard/BootstrapPage.h \
    Wizard/WizardPage.h \
    Widgets/ContextLabel.h \
    Widgets/SimilarArtistWidget.h \
    Widgets/PushButton.h \
    Widgets/TrackWidget.h \
    Dialogs/LicensesDialog.h \
    Widgets/ScrobblesListWidget.h \
    Widgets/ScrobblesWidget.h \
    Fingerprinter/Fingerprinter.h \
    Services/AnalyticsService.h \
    Services/AnalyticsService/AnalyticsService.h \
    Services/AnalyticsService/PersistentCookieJar.h \
    Settings/CheckFileSystemModel.h \
    Settings/CheckFileSystemView.h

contains(DEFINES, FFMPEG_FINGERPRINTING) {
    SOURCES += Fingerprinter/LAV_Source.cpp
    HEADERS += Fingerprinter/LAV_Source.h
}

mac:HEADERS += CommandReciever/CommandReciever.h \
                MediaKeys/MediaKey.h \
                ../../lib/3rdparty/SPMediaKeyTap/SPMediaKeyTap.h \
                ../../lib/3rdparty/SPMediaKeyTap/SPInvocationGrabbing/NSObject+SPInvocationGrabbing.h

mac:OBJECTIVE_SOURCES += CommandReciever/CommandReciever.mm \
                            Widgets/NothingPlayingWidget_mac.mm \
                            MediaKeys/MediaKey.mm \
                            ../../lib/3rdparty/SPMediaKeyTap/SPMediaKeyTap.m \
                            ../../lib/3rdparty/SPMediaKeyTap/SPInvocationGrabbing/NSObject+SPInvocationGrabbing.m

FORMS += \
    Widgets/PlaybackControlsWidget.ui \
    Dialogs/DiagnosticsDialog.ui \
    Widgets/MetadataWidget.ui \
    Settings/PreferencesDialog.ui \
    Settings/GeneralSettingsWidget.ui \
    Settings/AccountSettingsWidget.ui \
    Settings/IpodSettingsWidget.ui \
    Settings/ScrobbleSettingsWidget.ui \
    Settings/AdvancedSettingsWidget.ui \
    Wizard/FirstRunWizard.ui \
    Widgets/NothingPlayingWidget.ui \
    Widgets/FriendWidget.ui \
    Widgets/FriendListWidget.ui \
    Widgets/TrackWidget.ui \
    Dialogs/LicensesDialog.ui \
    Widgets/ScrobblesWidget.ui \
    Widgets/ProfileWidget.ui \
    Widgets/RadioWidget.ui

unix:!mac {
    CONFIG += qdbus

    SOURCES += MediaDevices/IpodDevice_linux.cpp \
               Mpris2/Mpris2.cpp \
               Mpris2/DBusAbstractAdaptor.cpp \
               Mpris2/MediaPlayer2.cpp \
               Mpris2/MediaPlayer2Player.cpp

    HEADERS += MediaDevices/IpodDevice_linux.h \
               Mpris2/Mpris2.h \
               Mpris2/DBusAbstractAdaptor.h \
               Mpris2/MediaPlayer2.h \
               Mpris2/MediaPlayer2Player.h
}

RESOURCES += \
    qrc/audioscrobbler.qrc

unix:!mac {
        target.path = $$BINDIR

        css.files = 'Last.fm Scrobbler.css'
        css.path  = $$DATADIR/lastfm-scrobbler

        desktop.files += lastfm-scrobbler.desktop
        desktop.path = $$DATADIR/applications

        icon16.files += icons/16x16/lastfm-scrobbler.png
        icon16.path = $$DATADIR/icons/hicolor/16x16/apps
        icon22.files += icons/22x22/lastfm-scrobbler.png
        icon22.path = $$DATADIR/icons/hicolor/22x22/apps
        icon32.files += icons/32x32/lastfm-scrobbler.png
        icon32.path = $$DATADIR/icons/hicolor/32x32/apps
        icon48.files += icons/48x48/lastfm-scrobbler.png
        icon48.path = $$DATADIR/icons/hicolor/48x48/apps
        icon64.files += icons/64x64/lastfm-scrobbler.png
        icon64.path = $$DATADIR/icons/hicolor/64x64/apps
        icon128.files += icons/128x128/lastfm-scrobbler.png
        icon128.path = $$DATADIR/icons/hicolor/128x128/apps

        INSTALLS += target css icon16 icon22 icon32 icon48 icon64 icon128 desktop
}
