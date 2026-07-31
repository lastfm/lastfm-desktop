Join us for chats on IRC!

Server: irc.last.fm
Channel: #last.desktop

# Build Dependencies

* Qt 5.15 (Qt4 is no longer supported)
* liblastfm >= 1.1 built with Qt5 (https://github.com/lastfm/liblastfm)

**Only the macOS build has been ported to Qt5 so far.** The Windows and
Linux sections below are historical: they describe the Qt4-era build and
those platforms need porting work (Qt4-only APIs and `Q_WS_*` era guards
remain in their platform code) before they build again. `qmake` will tell
you as much on those platforms.

## macOS

Builds natively on Apple Silicon (arm64) and Intel with Qt 5.15 from
Homebrew. Last verified with Qt 5.15.19, CMake 4.x and the macOS 26 SDK.
(Note: Homebrew has deprecated `qt@5` and plans to disable it in May 2027;
at that point this build will need a different Qt 5.15 source or a Qt 6
port.)

We recommend you have Xcode set as your build toolchain.

```
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
```

```
brew install qt@5 cmake pkg-config fftw libsamplerate
```

`fftw` and `libsamplerate` are needed by liblastfm's fingerprint library,
which its CMake build enables by default — pass `-DBUILD_FINGERPRINT=OFF` to
skip them. `ffmpeg` is only needed for `app/fingerprinter`, which is
currently disabled in `Last.fm.pro`.

### liblastfm

Clone liblastfm from https://github.com/lastfm/liblastfm parallel to your
lastfm-desktop checkout and build it with Qt5 (the default) into a local
install prefix:

```
cd liblastfm
mkdir _build && cd _build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@5) \
         -DCMAKE_INSTALL_PREFIX=$PWD/../_install \
         -DBUILD_TESTS=OFF \
         -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -j8
make install
ln -s lastfm5 ../_install/include/lastfm
```

(`CMAKE_POLICY_VERSION_MINIMUM` is needed because liblastfm still declares
`cmake_minimum_required(VERSION 2.8.6)`, which CMake 4 refuses outright.)

The desktop build looks for liblastfm in `../liblastfm/_install` relative to
the lastfm-desktop root. You can override this by passing
`LIBLASTFM_ROOT=/path/to/prefix` to qmake.

### Sparkle (auto-updates, optional)

If Sparkle.framework (2.x) is present in /Library/Frameworks or
~/Library/Frameworks at qmake time, the auto-updater is compiled in
(HAVE_SPARKLE); otherwise qmake prints a warning and the "Check for
Updates" menu item just opens the download page. To enable it, download a
Sparkle 2 release (last tested: 2.9.4) from
https://github.com/sparkle-project/Sparkle/releases and copy
Sparkle.framework into ~/Library/Frameworks, then re-run qmake.

Growl support still compiles if Growl.framework is present (HAVE_GROWL),
but the code path is unreachable at runtime — notifications always go
through Notification Center on the supported macOS versions. Don't bother.

Notes for release managers:

* Sparkle 2 dropped DSA appcast signatures. To ship updates to 2.2.x
  clients the appcast entries must be signed with an EdDSA key (add
  `SUPublicEDKey` to admin/dist/mac/Standard.plist and sign with Sparkle's
  generate_keys/sign_update tools); the bundled dsa_pub.pem only covers
  legacy 2.1.x clients.
* The feed the app actually checks is hardcoded in
  `lib/unicorn/Updater/Updater.h` (UPDATE_URL_MAC / UPDATE_URL_MAC_BETA) —
  the `SUFeedURL` in the Info.plist is overridden at startup.
* The updater still uses Sparkle 1.x's `SUUpdater` API via Sparkle 2's
  deprecated compatibility shim; port Updater_mac.mm to
  `SPUStandardUpdaterController` before Sparkle removes it.
* 2.2.0 raised the deployment target to macOS 11, and the Info.plist
  declares `LSMinimumSystemVersion` accordingly. Give appcast entries a
  `sparkle:minimumSystemVersion` so 2.1.x users on older macOS don't get
  offered an update they cannot run.

### API keys (optional)

If `LASTFM_API_KEY` / `LASTFM_API_SECRET` are exported when you run `make`
(they are expanded by make, not qmake — see lib/unicorn/unicorn.pro), they
are baked into the binary. If unset, the client falls back to the shared
public key in lib/unicorn/UnicornCoreApplication.cpp.

```
export LASTFM_API_KEY=your_api_key
export LASTFM_API_SECRET=your_api_secret
```

### Building

```
cd lastfm-desktop
$(brew --prefix qt@5)/bin/qmake -r
make -j8
open "_bin/Last.fm Scrobbler.app"
```

Note: `qt@5` is keg-only, so a bare `qmake` on your PATH is probably the
wrong one — always use the full path. For a clean rebuild, `rm -rf _bin`
and re-run qmake and make.

### Packaging

To make a locally runnable, self-contained bundle:

```
cp -R "_bin/Last.fm Scrobbler.app" dist-folder/
$(brew --prefix qt@5)/bin/macdeployqt "dist-folder/Last.fm Scrobbler.app" \
    -executable="dist-folder/Last.fm Scrobbler.app/Contents/Helpers/iPodScrobbler"

# macdeployqt deploys Qt frameworks and plain dylibs (including ours), but
# not third-party frameworks: Sparkle.framework is never copied. If you
# built with Sparkle, copy it in from wherever the build found it. No
# install_name_tool step is needed - the app already carries an
# @executable_path/../Frameworks rpath.
cp -R ~/Library/Frameworks/Sparkle.framework \
    "dist-folder/Last.fm Scrobbler.app/Contents/Frameworks/"

codesign --force --deep -s - "dist-folder/Last.fm Scrobbler.app"
```

Then verify the result really is self-contained — the dev build bakes
absolute rpaths to your checkout and Sparkle location into the binaries,
so a bundle with a missing framework still runs fine *on your machine* and
crashes at launch on everyone else's:

```
otool -l "dist-folder/Last.fm Scrobbler.app/Contents/MacOS/Last.fm Scrobbler" | grep -A2 LC_RPATH
ls "dist-folder/Last.fm Scrobbler.app/Contents/Frameworks/" | grep Sparkle
```

For actual distribution (not just local testing) you additionally need a
Developer ID signature with hardened runtime and notarization; sign nested
code inside-out rather than relying on the deprecated `--deep` flag, which
also clobbers Sparkle's own signatures.


## Windows (not yet ported to Qt5)

We used to build using Cygwin, but now we prefer not to.

You should get Windows version of the tool chain. Here are some recommendations.

- Git: http://code.google.com/p/msysgit/downloads/list
- CMake: http://www.cmake.org/cmake/resources/software.html
- pkg-config: http://www.gtk.org/download/win32.php
- Ruby: http://rubyinstaller.org/
- Perl: http://www.perl.org/get.html
- Win Platform SDK:http://www.microsoft.com/en-us/download/details.aspx?id=8279
- KDE Support: http://windows.kde.org/ Install the automoc and dbus packages.

### Qt

Install Qt binaries from either the Qt SDK or standalone binary package. You should be able to find everything here http://qt.nokia.com/downloads

You will also need the latest Windows SDK. We build using Visual Studio 2008.

### Winsparkle

This is the library we use to check for app updates. You should download the latest dll and headers form here http://winsparkle.org

This step should be optional really as most people will not want to add the update checking.

I found that I also needed to copy the dll into the lastfm-desktop/_bin folder. Create a pkg-config file for WinSparkle like this:

    Name: sparkle
    Description: Multimedia Library
    Version: 0.3
    Libs: -LC:/dev/Install/WinSparkle/Release -lWinSparkle
    Cflags: -IC:/dev/Install/WinSparkle/include

## Linux (not yet ported to Qt5)

On Debian or Ubuntu, you can download all the build dependencies by running:

    sudo apt-get install libavformat-dev libgpod-nogtk-dev liblastfm-dev \
                         libqt4-dev libqtwebkit-dev pkg-config \
                         zlib1g-dev

You should also install the `libqt4-sql-sqlite` plugin if you want to use the
software to scrobble your iPod.

# Build Instructions

    qmake -r
    make -j4
    
`make install` currently does not work on Windows or OSX.

Windows note: use nmake on Windows

Linux note: Linux users can set the install prefix like so `qmake PREFIX=/usr/local`

OSX note: if you installed Qt through homebrew it will default to a release build.

# Run Instructions

Apps are styled using stylesheets which are found in the source directory
of the app. By default the executable path is checked for the css file on
Windows and on OSX the bundle Resource directory is checked otherwise you'll
need to tell the app where the stylesheet is, like this: 

    ./Last.fm.exe -stylesheet path/to/Last.fm.css
    
On Linux, if you have not run `make install`, you can run the app like this
from the root of the source directory:

    _bin/lastfm-scrobbler -stylesheet app/client/Last.fm\ Scrobbler.css

# Build Support

We support developers trying to build the source on any platform. 

Seeing as we don't provide many varieties of binary package on Linux, we also
support non-developers building from source there. However within reason!
Please check around the net a little first. Ask your friends. Demand help
from people standing at bus-stops.

Maybe check the official forum: http://getsatisfaction.com/lastfm

# Bugs

If you find a bug in the software, please let us know about it.

Michael Coffey<michaelc@last.fm>
Desktop App Lead Developer, Last.fm
