Join us for chats on IRC!

Server: irc.last.fm
Channel: #last.desktop

# Build Dependencies

* Qt >= 4.8
* Ruby
* Perl
* liblastfm

You will also need depending on your chosen platform:-

## Mac OS X

### Homebrew

We recommend that you use Homebrew to install most of the dependancies:

    brew tap lastfm/lastfmdesktop
    brew install qt kde-phonon
    brew install --HEAD vlc
    brew install phonon-vlc

We install kde-phonon becuase the Phonon distributed with Qt is quite old.

Installing phonon-vlc will remove the QuickTime phonon plugin and replace
it with the VLC phonon plugin so that when we build the app we use .

### Other dependancies

You'll also need the Growl and libsparkle frameworks.

Get the latest Growl SDk from here http://code.google.com/p/growl/downloads/list

Get the latest Sparkle from here http://sparkle.andymatuschak.org/

Unzip both and put their frameworks in /Library/Frameworks/ so the build will find them.

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

### Phonon

We build the latest version of Phonon. This step is optional, if you're happy with the outdated one that ships with Qt.

You can clone Phonon from here git://anongit.kde.org/phonon

Follow their instructions to build it using cmake, etc. I found that CMake needed a little help wiht cmake-gui.

Create a pkg-config file for Phonon like this:

    Name: Phonon
    Description: Multimedia Library
    Version: 4.6.0
    Libs: -LC:/dev/Install/Phonon/lib C:/dev/Install/Phonon/lib/phonon.lib
    Cflags: -IC:/dev/Install/Phonon/include

### VLC

We use the VLC backend for phonon. This step is optional, if you are happy with the directx plugin that ships with Qt.

Download Latest VLC from http://www.videolan.org/vlc/download-windows.html

VLC is compiled with MinGW so we need to generate a .lib so that we can link to it with MSVC.

To do this you should follow the instructions here http://wiki.videolan.org/GenerateLibFromDll

- Copy the pkg-config files from 'INSTALL_DIR\VideoLAN\VLC\sdk\lib\pkgconfig' to your pkg-config search directory.

I found I also had to copy libvlc.dll and libvlccore.dll into lastfm-desktop/_bin

### Phonon-VLC

This is the Qt plugin that gets Phonon to use libvlc. Again, this is an optional.

You can clone phonon-vlc from here git://anongit.kde.org/phonon-vlc

Follow there instructions for building it. I found that CMake needed a little help wiht cmake-gui.

You should copy the resulting `phonon_vlc.dll` into `lastfm-desktop/_bin/plugins/phonon_backend`

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

* pkg-config
* taglib
* libsamplerate
* fftw3
* libmad
* libavformat
* libgpod
* sqlite3

Possibly you'll have to play around to get a phonon that works with KDE4 and 
Qt. I can't help you there I'm afraid. An example Ubuntu installation would 
probably go:

    sudo apt-get install libqt4-dev pkg-config libtag1-dev libsamplerate0-dev \
                         libfftw3-dev libmad0-dev libsqlite3-dev libqt4-sql-sqlite \
                         libavformat-dev libgpod-dev libphonon-dev phonon

# Build Instructions

    qmake -r
    make -j4
    
`make install` currently does nothing, so running the app is a bit tricky (see below).

Note: use nmake on Windows

Note that if you installed Qt through homebrew it will default to a release build.

# Run Instructions

Apps are styled using stylesheets which are found in the source directory
of the app. By default the executable path is checked for the css file on
Windows and on OSX the bundle Resource directory is checked otherwise you'll
need to tell the app where the stylesheet is, like this: 

    ./Last.fm.exe -stylesheet path/to/Last.fm.css
    
Or on Linux,from the root of the source directory:

    LD_LIBRARY_PATH=_bin:/usr/local/lib/x86_64-linux-gnu _bin/Last.fm\ Scrobbler -stylesheet app/client/Last.fm\ Scrobbler.css

# Build Support

We support developers trying to build the source on any platform. 

Seeing as we don't provide many varieties of binary package on Linux, we also
support non-developers building from source there. However within reason!
Please check around the net a little first. Ask your friends. Demand help
from people standing at bus-stops.

Maybe check the official forum: http://www.last.fm/forum/34905

# Bugs

If you find a bug in the software, please let us know about it.

Michael Coffey<michaelc@last.fm>
Desktop App Lead Developer, Last.fm
