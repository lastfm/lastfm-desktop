TARGET = listener
TEMPLATE = lib
QT = core xml network
CONFIG += unicorn logger
unix:!mac {
    CONFIG += staticlib
    QMAKE_DISTCLEAN += -f ../../_bin/liblistener.a
}

# basically not easy to support on other platforms, but feel free to fork
unix:!mac {
    QT += dbus
    SOURCES += DBusListener.cpp
    HEADERS += DBusListener.h
}

include( ../../admin/include.qmake )

DEFINES += _LISTENER_DLLEXPORT LASTFM_COLLAPSE_NAMESPACE

win32:LIBS += Advapi32.lib User32.lib

SOURCES += \
	PlayerMediator.cpp \
	PlayerListener.cpp \
	PlayerConnection.cpp \
	PlayerCommandParser.cpp \
        legacy/LegacyPlayerListener.cpp

HEADERS += \
	State.h \
	PlayerMediator.h \
	PlayerListener.h \
	PlayerConnection.h \
	PlayerCommandParser.h \
	PlayerCommand.h \
        legacy/LegacyPlayerListener.h

unix:!mac {
    SOURCES -= legacy/LegacyPlayerListener.cpp
    HEADERS -= legacy/LegacyPlayerListener.h
}

mac {
    SOURCES += mac/ITunesListener.cpp

    OBJECTIVE_SOURCES += mac/SpotifyListener.mm

    HEADERS += mac/ITunesListener.h \
                    mac/SpotifyListener.h

    LIBS += -weak_framework AppKit
}

win32 {
    SOURCES += win/SpotifyListener.cpp \
               win/NamedPipeServer.cpp \
               ../../plugins/iTunes/ITunesTrack.cpp \
               ../../plugins/iTunes/ITunesComWrapper.cpp \
               $$ROOT_DIR/plugins/scrobsub/EncodingUtils.cpp \
               $$ROOT_DIR/lib/3rdparty/iTunesCOMAPI/iTunesCOMInterface_i.c

    HEADERS += win/SpotifyListener.h \
               win/NamedPipeServer.h \
               ../../plugins/iTunes/ITunesTrack.h \
               ../../plugins/iTunes/ITunesComWrapper.h \
               ../../plugins/iTunes/ITunesEventInterface.h \
               ../../plugins/iTunes/ITunesExceptions.h \
               $$ROOT_DIR/plugins/scrobsub/EncodingUtils.h

    LIBS += -lcomsuppw

    DEFINES += _WIN32_DCOM
}
