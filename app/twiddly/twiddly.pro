TARGET = iPodScrobbler
LIBS += -lunicorn
QT = core xml sql

CONFIG += lastfm logger
CONFIG -= app_bundle

include( ../../admin/include.qmake )

# TODO: FIX THIS: I think this means that we can only build bundles
mac {
    DESTDIR = "../../_bin/Last.fm Scrobbler.app/Contents/Helpers"
}

DEFINES += LASTFM_COLLAPSE_NAMESPACE
SOURCES = main.cpp \
          TwiddlyApplication.cpp \
          PlayCountsDatabase.cpp \
          IPod.cpp \
          Utils.cpp

HEADERS = TwiddlyApplication.h \
          PlayCountsDatabase.h \
          IPod.h \
          Utils.h

mac {
    SOURCES += ITunesLibrary_mac.cpp
    OBJECTIVE_SOURCES += Utils_mac.mm
}

win32 {
    # Would prefer to refer to ITunesTrack.cpp and ITunesComWrapper.cpp in-situ
    # in the ../../plugins/iTunes/ directory, but that triggers bugs in nmake
    # causing it to compile the wrong main.cpp and IPod.cpp!
    # So here we are copying them and their dependencies.
    # Oh, and for some reason, cygwin mutilates their permissions.
    
    system( xcopy ..\\..\\plugins\\iTunes\\ITunesTrack.cpp . /Y /Q )
    system( xcopy ..\\..\\plugins\\iTunes\\ITunesTrack.h . /Y /Q )
    system( xcopy ..\\..\\plugins\\iTunes\\ITunesExceptions.h . /Y /Q )
    system( xcopy ..\\..\\plugins\\iTunes\\ITunesComWrapper.cpp . /Y /Q )
    system( xcopy ..\\..\\plugins\\iTunes\\ITunesComWrapper.h . /Y /Q )
    system( xcopy ..\\..\\plugins\\iTunes\\ITunesEventInterface.h . /Y /Q )

    SOURCES += ITunesLibrary_win.cpp \
               ITunesTrack.cpp \
               ITunesComWrapper.cpp \
               $$ROOT_DIR/plugins/scrobsub/EncodingUtils.cpp \
               $$ROOT_DIR/lib/3rdparty/iTunesCOMAPI/iTunesCOMInterface_i.c

    HEADERS += ITunesTrack.h \
               ITunesComWrapper.h \
               ITunesEventInterface.h \
               ITunesExceptions.h \
               $$ROOT_DIR/plugins/scrobsub/EncodingUtils.h

    LIBS += -lcomsuppw

    DEFINES += _WIN32_DCOM
    RC_FILE = Twiddly.rc
}
