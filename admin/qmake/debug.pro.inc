CONFIG( debug, debug|release ) {
    mac* {
        #speeds up debug builds by only compiling x86
        CONFIG -= ppc
    }

    unix:!mac {
        QMAKE_CXXFLAGS_DEBUG = -ggdb
    }

    VERSION_UPDATE_PATTERN = *.*.*.*
}
else {
    CONFIG += warn_off
    DEFINES += NDEBUG
    #macx*:CONFIG += ppc x86
}
