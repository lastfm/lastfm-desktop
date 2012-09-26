TARGET = listener
TEMPLATE = lib
QT = core xml network
CONFIG += unicorn logger

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
	PlayerCommandParser.cpp

HEADERS += \
	State.h \
	PlayerMediator.h \
	PlayerListener.h \
	PlayerConnection.h \
	PlayerCommandParser.h \
	PlayerCommand.h

mac {
    SOURCES += mac/ITunesListener.cpp

    OBJECTIVE_SOURCES += mac/SpotifyListener.mm

    HEADERS += mac/ITunesListener.h \
                    mac/SpotifyListener.h

    LIBS += -framework AppKit
}

win32 {
    SOURCES += win/SpotifyListener.cpp \
               ../../plugins/iTunes/ITunesTrack.cpp \
               ../../plugins/iTunes/ITunesComWrapper.cpp \
               $$ROOT_DIR/plugins/scrobsub/EncodingUtils.cpp \
               $$ROOT_DIR/lib/3rdparty/iTunesCOMAPI/iTunesCOMInterface_i.c

    HEADERS += win/SpotifyListener.h \
               ../../plugins/iTunes/ITunesTrack.h \
               ../../plugins/iTunes/ITunesComWrapper.h \
               ../../plugins/iTunes/ITunesEventInterface.h \
               ../../plugins/iTunes/ITunesExceptions.h \
               $$ROOT_DIR/plugins/scrobsub/EncodingUtils.h

    LIBS += -lcomsuppw

    DEFINES += _WIN32_DCOM
}
