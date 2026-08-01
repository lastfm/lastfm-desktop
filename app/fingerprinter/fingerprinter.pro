TARGET = fingerprinter
QT = core network xml sql

CONFIG += lastfm unicorn logger fingerprint ffmpeg
CONFIG -= app_bundle

include( ../../admin/include.qmake )

# TODO: FIX THIS: I think this means that we can only build bundles
mac {
    DESTDIR = "../../_bin/Last.fm Scrobbler.app/Contents/Helpers"
    # dylib bundling is handled by macdeployqt now, not bundleFrameworks.sh
}

SOURCES += main.cpp \
            Fingerprinter.cpp \
            LAV_Source.cpp

HEADERS += LAV_Source.h \
            Fingerprinter.h





DEFINES += LASTFM_COLLAPSE_NAMESPACE LASTFM_FINGERPRINTER

