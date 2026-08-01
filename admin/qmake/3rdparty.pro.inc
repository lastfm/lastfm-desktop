mac {
    # Homebrew: /opt/homebrew on Apple Silicon, /usr/local on Intel
    LIBS += -L/opt/homebrew/lib -L/usr/local/lib
    INCLUDEPATH += /opt/homebrew/include /usr/local/include

    # liblastfm built parallel to this repo (Qt5 build, see README)
    isEmpty(LIBLASTFM_ROOT):LIBLASTFM_ROOT = $$ROOT_DIR/../liblastfm/_install
    exists( $$LIBLASTFM_ROOT/lib ) {
        LIBS += -L$$LIBLASTFM_ROOT/lib
        INCLUDEPATH += $$LIBLASTFM_ROOT/include
    }
}

CONFIG( taglib ) {
    CONFIG += link_pkgconfig
    PKGCONFIG += taglib
    win32:debug:LIBS += tagd.lib    # tagd.lib will precede PKGCONFIG added libs
}

CONFIG( ffmpeg ) {
    mac {
        LIBS += /usr/local/lib/libavcodec.dylib
        LIBS += /usr/local/lib/libavformat.dylib
        LIBS += /usr/local/lib/libavutil.dylib
        LIBS += /usr/local/lib/libswresample.dylib
        DEFINES += HAVE_SWRESAMPLE
    } else {
        CONFIG += link_pkgconfig
        PKGCONFIG += libavformat libavcodec libavutil
        packagesExist(libswresample) {
            PKGCONFIG += libswresample
            DEFINES += HAVE_SWRESAMPLE
        }
        packagesExist(libavresample) {
            PKGCONFIG += libavresample
            DEFINES += HAVE_AVRESAMPLE
        }
    }
}


CONFIG( boost ) {
    mac:CONFIG(app_bundle){
        LIBS += /usr/local/lib/libboost_thread-mt.a
    }
    else:unix {
        LIBS += -lboost_thread-mt
    }
}

CONFIG( break ) {
    win32{
        CONFIG += link_pkgconfig
        PKGCONFIG += breakpad
        LIBS += wininet.lib
    }
    else:mac{
        LIBS += -framework Breakpad
    }
}

CONFIG( sparkle ) {
    win32{
        CONFIG += link_pkgconfig

        CONFIG(debug, debug|release) {
            PKGCONFIG += qtsparkle-debug
        }
        else {
            PKGCONFIG += qtsparkle-release
        }
    }
    else:mac{
        # Sparkle from https://github.com/sparkle-project/Sparkle/releases
        # installed into either framework search location; if it isn't there
        # the updater compiles as a no-op (see Updater_mac.mm). Deliberately
        # pinned to 1.27.x for now so the existing DSA-signed appcast keeps
        # working - see the README release-manager notes before upgrading.
        SPARKLE_DIR =
        exists( /Library/Frameworks/Sparkle.framework ):SPARKLE_DIR = /Library/Frameworks
        else:exists( $$(HOME)/Library/Frameworks/Sparkle.framework ):SPARKLE_DIR = $$(HOME)/Library/Frameworks

        !isEmpty( SPARKLE_DIR ) {
            message( "Sparkle found in $$SPARKLE_DIR - auto-update enabled" )
            DEFINES += HAVE_SPARKLE
            QMAKE_CXXFLAGS += -F$$SPARKLE_DIR
            QMAKE_OBJECTIVE_CFLAGS += -F$$SPARKLE_DIR
            LIBS += -F$$SPARKLE_DIR -framework Sparkle
            # Sparkle's install name is @rpath/Sparkle.framework/..., so both
            # dev runs and macdeployqt need this on the rpath to resolve it.
            # NB an absolute developer path baked into the binary - macdeployqt
            # strips it from the app, see the README packaging notes.
            QMAKE_RPATHDIR += $$SPARKLE_DIR
        } else {
            warning( "Sparkle.framework not found - the auto-updater will be compiled OUT of this build" )
        }
    }
}

CONFIG( sqlite3 ) {
    win32 {
        CONFIG += link_pkgconfig
        PKGCONFIG += sqlite3
    }
    else:LIBS += -lsqlite3
}

CONFIG( yajl ) {
	win32 {
		CONFIG += link_pkgconfig
		debug:PKGCONFIG += yajl-debug
		release:PKGCONFIG += yajl-release
	}
	else:CONFIG(app_bundle) {
                LIBS += /usr/local/lib/libyajl_s.a
	}
	else:LIBS += -lyajl
}

