TEMPLATE = subdirs
CONFIG += ordered
# The Qt5 port has only been done for macOS so far. The Windows and Linux
# code still contains Qt4-era Q_WS_WIN/Q_WS_X11 guards (and Qt4-only APIs)
# that need porting before those builds will work again.
!mac:error( "The Qt5 port currently supports macOS only. See README.md." )

SUBDIRS = lib/logger \
          lib/unicorn \
          lib/listener \
          i18n \
          app/client \
          app/twiddly
          #app/fingerprinter
          #app/boffin

unix:!mac:SUBDIRS -= app/twiddly

CONFIG( tests ) {
    SUBDIRS += \
        lib/lastfm/core/tests/test_libcore.pro \
        lib/lastfm/types/tests/test_libtypes.pro \
        lib/lastfm/scrobble/tests/test_libscrobble.pro \
        lib/listener/tests/test_liblistener.pro
}
