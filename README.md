Join us for chats on IRC!

Server: irc.last.fm
Channel: #last.desktop

# Build Dependencies

* Qt >= 4.8
* liblastfm >= 1.0.7

You will also need depending on your chosen platform:-

## Mac OS X

### Homebrew

We recommend that you use Homebrew to install most of the dependencies.

We recommend you have XCode set as your build toolchain.

```
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

```

```
brew install ffmpeg coreutils cmake fftw libsamplerate
```


We recommend Qt 4.8.7, the last version with Webkit support.

```
brew install cartr/qt4/qt@4
brew install cartr/qt4/qt-webkit@2.3
```

### liblastfm

Download liblastfm from https://github.com/lastfm/liblastfm parallel to the build of lastfm-desktop.

As the Desktop Client supports only Qt4 at the moment, you will need to set it to Qt4 mode.

In CMakeLists.txt, change
```
option(BUILD_WITH_QT4 "Build liblastfm with Qt4" ON)

```

Then build and make.
```
cd liblastfm
mkdir _build && cd _build
cmake ..
make -j4
make install
```

### Other dependencies

You'll also need the Growl and libsparkle frameworks.

Get the latest Growl SDk from here http://code.google.com/p/growl/downloads/list - latest tested 1.2.2

Get the latest Sparkle from here http://sparkle.andymatuschak.org/ - latest tested 1.21.3

Unzip both and put their frameworks in /Library/Frameworks/ so the build will find them.

You may need to symlink the headers files into the lastfm-desktop directory:

```
ln -s /Library/Frameworks/Sparkle.framework/Headers Sparkle
ln -s /Library/Frameworks/Growl.framework/Headers Growl
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
