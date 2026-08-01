CONFIG( unicorn ) {
    LIBS += -lunicorn
    QT += gui widgets
    CONFIG += lastfm
}
CONFIG( listener ) {
    LIBS += -llistener
    CONFIG += lastfm
}
CONFIG( fingerprint ) {
    LIBS += -llastfm_fingerprint5
    CONFIG += lastfm
}
CONFIG( logger ) {
    LIBS += -llogger
    CONFIG += lastfm
}
CONFIG( lastfm ) {
    QT += core network xml
    LIBS += -llastfm5
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
        # Notification Center support. This used to be gated behind
        # CONFIG(notifications), which nothing ever set, so the NSUserNotification
        # path had never actually been compiled before the Qt5 port.
        # NSUserNotification is deprecated since macOS 11 - port
        # lib/unicorn/notify/Notify.mm to UNUserNotificationCenter before an
        # SDK finally drops it.
        DEFINES += LASTFM_USER_NOTIFICATIONS

        LIBS += -weak_framework Carbon -weak_framework CoreFoundation -weak_framework Foundation
    }
}
