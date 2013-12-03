TARGET = fingerprinter
QT = core network xml sql

CONFIG += lastfm unicorn logger fingerprint ffmpeg
CONFIG -= app_bundle

include( ../../admin/include.qmake )

# TODO: FIX THIS: I think this means that we can only build bundles
mac {
    DESTDIR = "../../_bin/Last.fm Scrobbler.app/Contents/Helpers"
    QMAKE_POST_LINK += ../../admin/dist/mac/bundleFrameworks.sh \"$$DESTDIR/$$TARGET\"
}

SOURCES += main.cpp \
            Fingerprinter.cpp \
            LAV_Source.cpp

HEADERS += LAV_Source.h \
            Fingerprinter.h





DEFINES += LASTFM_COLLAPSE_NAMESPACE LASTFM_FINGERPRINTER

