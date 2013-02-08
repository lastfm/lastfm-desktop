CONFIG( unicorn ) {
    LIBS += -lunicorn
    QT += gui
    CONFIG += lastfm
}
CONFIG( listener ) {
    LIBS += -llistener
    CONFIG += lastfm
}
CONFIG( fingerprint ) {
    LIBS += -llastfm_fingerprint
    CONFIG += lastfm
}
CONFIG( logger ) {
    LIBS += -llogger
    CONFIG += lastfm
}
CONFIG( lastfm ) {
    QT += core network xml
    LIBS += -llastfm
    win32 {
        CONFIG += link_pkgconfig

        CONFIG(debug, debug|release) {
            PKGCONFIG += lastfm-debug
        }
        else {
            PKGCONFIG += lastfm-release
        }

    }
    mac{
        CONFIG( notifications ) {
            MAC_SDK  = $$system(xcode-select --print-path)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk

            if( !exists( $$MAC_SDK) ) {
                message("You do not have the MacOSX10.8 SDK installed. User Notifications will not be built.")
            }
            else {
                macx:QMAKE_MAC_SDK = $$MAC_SDK
                DEFINES += LASTFM_USER_NOTIFICATIONS
            }
        }

        LIBS += -weak_framework Carbon -weak_framework CoreFoundation -weak_framework Foundation
    }
}
