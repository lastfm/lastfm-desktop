include( ../admin/include.qmake )

TRANSLATIONS = lastfm_de.ts \
               lastfm_en.ts \
               lastfm_es.ts \
               lastfm_fr.ts \
               lastfm_it.ts \
               lastfm_ja.ts \
               lastfm_pl.ts \
               lastfm_pt.ts \
               lastfm_ru.ts \
               lastfm_sv.ts \
               lastfm_tr.ts \
               lastfm_zh_CN.ts

isEmpty(vcproj) {
    QMAKE_LINK = @: IGNORE THIS LINE
    OBJECTS_DIR =
    win32:CONFIG -= embed_manifest_exe
} else {
    CONFIG += console
    PHONY_DEPS = .
    phony_src.input = PHONY_DEPS
    phony_src.output = phony.c
    phony_src.variable_out = GENERATED_SOURCES
    phony_src.commands = echo int main() { return 0; } > phony.c
    phony_src.name = CREATE phony.c
    phony_src.CONFIG += combine
    QMAKE_EXTRA_COMPILERS += phony_src
}

isEmpty(QMAKE_LRELEASE) {
    win32|os2:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    unix {
        !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease-qt4 }
    } else {
        !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease }
    }
}

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent -removeidentical ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

unix:!mac {
        i18n.files += *.qm
        i18n.path = $$DATADIR/lastfm-scrobbler/i18n
        i18n.CONFIG += no_check_exist

        INSTALLS += i18n
}
