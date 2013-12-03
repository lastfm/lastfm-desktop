TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = lib/logger \
          lib/unicorn \
          lib/listener \
          i18n \
          app/client \
          app/twiddly \ 
          app/fingerprinter
          #app/boffin

unix:!mac:SUBDIRS -= app/twiddly

CONFIG( tests ) {
    SUBDIRS += \
        lib/lastfm/core/tests/test_libcore.pro \
        lib/lastfm/types/tests/test_libtypes.pro \
        lib/lastfm/scrobble/tests/test_libscrobble.pro \
        lib/listener/tests/test_liblistener.pro
}
