#!/bin/bash
# Build a universal (x86_64 + arm64) Qt 5.15 for the Last.fm scrobbler.
#
# Homebrew only ships single-arch Qt, so a universal app needs a Qt built
# from source with both architectures. This script reuses Homebrew's Qt
# source *with its modern-SDK patches applied* (brew unpack --patch), then
# configures it universal. Only qtbase and qttools are built - all the
# scrobbler needs (and qttools provides macdeployqt + lrelease).
#
# Takes roughly an hour and ~10GB of scratch space. Afterwards:
#
#   liblastfm:  cmake .. -DCMAKE_PREFIX_PATH=$PREFIX \
#                        -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" ...
#   scrobbler:  $PREFIX/bin/qmake -r CONFIG+=universal && make
#
set -euo pipefail

PREFIX="${QT_UNIVERSAL_PREFIX:-$HOME/Qt/5.15-universal}"
WORK="${QT_UNIVERSAL_WORK:-/private/tmp/qt5-universal-src}"
LOG="${QT_UNIVERSAL_LOG:-/tmp/qt5-universal-build.log}"

echo "Unpacking Homebrew's patched qt@5 source into $WORK" | tee -a "$LOG"
mkdir -p "$WORK"
if ! ls "$WORK"/qt*/configure >/dev/null 2>&1; then
    brew unpack --patch --destdir "$WORK" qt@5 2>&1 | tee -a "$LOG"
fi

SRC=$(ls -d "$WORK"/qt*/ | head -1)
cd "$SRC"

# The macOS 26 SDK removed the ancient AGL framework, which Qt 5.15's darwin
# makespec still references; configure's OpenGL test fails on it. OpenGL
# itself is still present (deprecated), so just drop AGL.
sed -i '' \
    -e 's|    /System/Library/Frameworks/OpenGL.framework/Headers \\|    /System/Library/Frameworks/OpenGL.framework/Headers|' \
    -e '\|    /System/Library/Frameworks/AGL.framework/Headers/|d' \
    -e 's|QMAKE_LIBS_OPENGL       = -framework OpenGL -framework AGL|QMAKE_LIBS_OPENGL       = -framework OpenGL|' \
    qtbase/mkspecs/common/mac.conf

# Skip every submodule except qtbase and qttools
SKIPS=""
for mod in qt*/; do
    mod="${mod%/}"
    case "$mod" in
        qtbase|qttools) ;;
        qt*) SKIPS="$SKIPS -skip $mod" ;;
    esac
done

echo "Configuring universal Qt into $PREFIX" | tee -a "$LOG"
rm -f config.cache
# qdoc and clang-based lupdate link against libclang, which Homebrew ships
# single-arch - and we don't need documentation tooling anyway
./configure -prefix "$PREFIX" \
    -opensource -confirm-license -release \
    QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64" \
    -nomake examples -nomake tests \
    -no-openssl -securetransport \
    -no-feature-qdoc \
    $SKIPS 2>&1 | tee -a "$LOG"

echo "Building (this is the long part)" | tee -a "$LOG"
make -j"$(sysctl -n hw.ncpu)" 2>&1 | tee -a "$LOG"

echo "Installing to $PREFIX" | tee -a "$LOG"
make install 2>&1 | tee -a "$LOG"

echo "Done. Sanity check:" | tee -a "$LOG"
lipo -info "$PREFIX/lib/QtCore.framework/QtCore" | tee -a "$LOG"
