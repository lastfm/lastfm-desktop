Join us for chats on IRC!

Server: irc.last.fm
Channel: #last.desktop

# Build Dependencies

* Qt 5.15 (the client app; Qt4 is no longer supported)
* liblastfm >= 1.1 built with Qt5 (https://github.com/lastfm/liblastfm)

You will also need depending on your chosen platform:-

## macOS

Builds natively on Apple Silicon (arm64) and Intel with Qt 5.15 from Homebrew.

We recommend you have Xcode set as your build toolchain.

```
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
```

```
brew install qt@5 cmake pkg-config
```

(`ffmpeg`, `fftw` and `libsamplerate` are only needed if you also build the
fingerprinter, which is currently disabled in Last.fm.pro.)

### liblastfm

Clone liblastfm from https://github.com/lastfm/liblastfm parallel to your
lastfm-desktop checkout and build it with Qt5 (the default) into a local
install prefix:

```
cd liblastfm
mkdir _build && cd _build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@5) \
         -DCMAKE_INSTALL_PREFIX=$PWD/../_install \
         -DBUILD_TESTS=OFF -DBUILD_DEMOS=OFF \
         -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -j8
make install
ln -s lastfm5 ../_install/include/lastfm
```

The desktop build looks for liblastfm in `../liblastfm/_install` relative to
the lastfm-desktop root. You can override this by passing
`LIBLASTFM_ROOT=/path/to/prefix` to qmake.

### Optional frameworks

Growl and Sparkle are optional these days. Track notifications use the native
macOS notification centre. If Sparkle.framework (2.x) is present in either
/Library/Frameworks or ~/Library/Frameworks the auto-updater is compiled in
(HAVE_SPARKLE), otherwise it is a no-op. Likewise Growl.framework enables the
legacy Growl path (HAVE_GROWL), which you almost certainly don't want.

To enable the updater, download the latest Sparkle 2 release from
https://github.com/sparkle-project/Sparkle/releases and copy
Sparkle.framework into ~/Library/Frameworks.

Note for release managers: Sparkle 2 dropped DSA appcast signatures. To ship
updates through Sparkle 2 clients the appcast entries must be signed with an
EdDSA key (add SUPublicEDKey to admin/dist/mac/Standard.plist and sign with
Sparkle's generate_keys/sign_update tools); the bundled dsa_pub.pem only
covers legacy 2.1.x clients.

### API keys

The build bakes your Last.fm API credentials in from the environment at
compile time, so export these before running make:

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

### Packaging

To make a self-contained, distributable bundle:

```
cp -R "_bin/Last.fm Scrobbler.app" dist-folder/
$(brew --prefix qt@5)/bin/macdeployqt "dist-folder/Last.fm Scrobbler.app" \
    -executable="dist-folder/Last.fm Scrobbler.app/Contents/Helpers/iPodScrobbler"
# macdeployqt does not follow @rpath deps of our dylibs, so if you built with
# Sparkle, copy the framework in yourself before signing:
cp -R ~/Library/Frameworks/Sparkle.framework \
    "dist-folder/Last.fm Scrobbler.app/Contents/Frameworks/"
codesign --force --deep -s - "dist-folder/Last.fm Scrobbler.app"   # or your Developer ID
```

### Now you're ready!

```
cd lastfm-desktop
rm -r _bin
qmake -r
make clean
make -j4
```


## Windows

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

## Linux

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
