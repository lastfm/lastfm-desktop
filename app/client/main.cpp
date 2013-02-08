/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QtGlobal>

#ifdef Q_OS_MAC64
    #include <Carbon/Carbon.h>
    static pascal OSErr appleEventHandler( const AppleEvent*, AppleEvent*, void* );
#elif defined Q_OS_MAC32
    #include </System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/AE.framework/Versions/A/Headers/AppleEvents.h>
    static pascal OSErr appleEventHandler( const AppleEvent*, AppleEvent*, long );
#endif

#include <QShortcut>
#include <QKeySequence>
#include <QStringList>
#include <QRegExp>

#include "Application.h"
#include "ScrobSocket.h"
#include "lib/unicorn/UnicornApplication.h"
#include "lib/unicorn/qtsingleapplication/qtsinglecoreapplication.h"
#include "lib/unicorn/UnicornSettings.h"
#include "Services/ScrobbleService.h"
#include "Services/RadioService.h"

#include "lib/unicorn/CrashReporter/CrashReporter.h"

void cleanup();


namespace lastfm
{
    extern LASTFM_DLLEXPORT QByteArray UserAgent;
}

int main( int argc, char** argv )
{
    //unicorn::CrashReporter* crashReporter = new unicorn::CrashReporter;

    QtSingleCoreApplication::setApplicationName( "Last.fm Scrobbler" );
    QtSingleCoreApplication::setOrganizationName( "Last.fm" );
    QtSingleCoreApplication::setApplicationVersion( APP_VERSION );

    // ATTENTION! Under no circumstance change these strings! --mxcl
#ifdef WIN32
    lastfm::UserAgent = "Last.fm Client " APP_VERSION " (Windows)";
#elif __APPLE__
    lastfm::UserAgent = "Last.fm Client " APP_VERSION " (OS X)";
#elif defined (Q_WS_X11)
    lastfm::UserAgent = "Last.fm Client " APP_VERSION " (X11)";
#endif

    try
    {
        audioscrobbler::Application app( argc, argv );

#ifdef Q_OS_WIN32
        QStringList args = app.arguments();
#else
        QStringList args = app.arguments().mid( 1 );
#endif

        if ( !args.contains( "--new" ) )
        {
            if ( app.sendMessage( args ) || args.contains("--exit") )
                return 0;

            // It's possible that we were unable to send the
            // message, but the app is actually running
            if ( app.isRunning() )
                return 0;

        }

        qAddPostRoutine(cleanup);

#ifdef Q_OS_MAC
        AEEventHandlerUPP h = NewAEEventHandlerUPP( appleEventHandler );
        AEInstallEventHandler( 'GURL', 'GURL', h, 0, false );
#endif

        app.init();
        app.parseArguments( args );
        return app.exec();
    }
    catch (std::exception& e)
    {
        qDebug() << "unhandled exception " << e.what();
    }
    catch (unicorn::Application::StubbornUserException&)
    {
        // user wouldn't log in
        return 0;
    }

    //delete crashReporter;
}

#ifdef Q_OS_MAC
#ifdef Q_OS_MAC64
static pascal OSErr appleEventHandler( const AppleEvent* e, AppleEvent*, void* )
#elif defined Q_OS_MAC32
static pascal OSErr appleEventHandler( const AppleEvent* e, AppleEvent*, long )
#endif //Q_OS_MAC64/32

{
    OSType id = typeWildCard;
    AEGetAttributePtr( e, keyEventIDAttr, typeType, 0, &id, sizeof(id), 0 );
    
    switch (id)
    {
        case 'GURL':
        {
            DescType type;
            Size size;

            char buf[1024];
            AEGetParamPtr( e, keyDirectObject, typeChar, &type, &buf, 1023, &size );
            buf[size] = '\0';

            RadioService::instance().play( RadioStation( QString::fromUtf8( buf ) ) );
            return noErr;
        }
            
        default:
            return unimpErr;
    }
}
#endif //Q_OS_MAC


void cleanup()
{
    if (RadioService::instance().audioOutput()) {
	    unicorn::AppSettings().setValue( "Volume", RadioService::instance().audioOutput()->volume() );
    }
}
