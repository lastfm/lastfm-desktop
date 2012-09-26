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

#include <QDebug>
#include <QLocale>

#include <lastfm/ws.h>
#include <lastfm/misc.h>

#include "UnicornCoreApplication.h"

#include "common/c++/Logger.h"

using namespace lastfm;

#ifdef WIN32
extern void qWinMsgHandler( QtMsgType t, const char* msg );
#endif
#ifdef __APPLE__
static QLocale qMacLocale();
#endif


unicorn::CoreApplication::CoreApplication( int& argc, char** argv )
                      : QtSingleCoreApplication( argc, argv )
{
    init();
}

void //static
unicorn::CoreApplication::init()
{
    QCoreApplication::setOrganizationName( "Last.fm" /*unicorn::organizationName() */ );
    QCoreApplication::setOrganizationDomain( "last.fm" /*unicorn::organizationDomain()*/ );

    // you can override this api key and secret by setting the
    // environment variables LASTFM_API_KEY and LASTFM_API_SECRET
    lastfm::ws::ApiKey = QString( API_KEY ).isEmpty() ? "9e89b44de1ff37c5246ad0af18406454" : API_KEY;
    lastfm::ws::SharedSecret = QString( API_SECRET ).isEmpty() ? "147320ea9b8930fe196a4231da50ada4" : API_SECRET;

    dir::runtimeData().mkpath( "." );
#ifndef WIN32
    QFile runtimeDataPerms( dir::runtimeData().absolutePath() );
    runtimeDataPerms.setPermissions( QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner );
#endif
    dir::cache().mkpath( "." );
    dir::logs().mkpath( "." );

#ifdef WIN32
    QString bytes = CoreApplication::log( applicationName() ).absoluteFilePath();
    const wchar_t* path = bytes.utf16();
#else
    QByteArray bytes = CoreApplication::log( applicationName() ).absoluteFilePath().toLocal8Bit();
    const char* path = bytes.data();
#endif
    new Logger( path );

    qInstallMsgHandler( qMsgHandler );
    qDebug() << "Introducing" << applicationName()+' '+applicationVersion();
    qDebug() << "Directed by" << lastfm::platform();
}


void
unicorn::CoreApplication::qMsgHandler( QtMsgType type, const char* msg )
{
#ifndef NDEBUG
#ifdef WIN32
    qWinMsgHandler( type, msg );
#else
    Q_UNUSED( type );
    fprintf( stderr, "%s\n", msg );
    fflush( stderr );
#endif
#endif

    static int spam = 0;
    static QByteArray previous_msg;
    
    if (previous_msg == msg) {
        ++spam;
        return;
    }
    
    if (spam) {
        // +1 so as to include first duplication too
        Logger::the().log( QString( "Times above line spammed: %L1").arg( spam + 1 ).toUtf8() );
        spam = 0;
    }
    
    previous_msg = msg;    
    Logger::the().log( msg );
}


QFileInfo
unicorn::CoreApplication::log( const QString& productName )
{
#ifdef NDEBUG
    return dir::logs().filePath( productName + ".log" );
#else
    return dir::logs().filePath( productName + ".debug.log" );
#endif
}


#ifdef __APPLE__
#include <Carbon/Carbon.h>
static QLocale qMacLocale()
{
    //TODO see what Qt's version does
    CFArrayRef languages = (CFArrayRef) CFPreferencesCopyValue( 
            CFSTR( "AppleLanguages" ),
            kCFPreferencesAnyApplication,
            kCFPreferencesCurrentUser,
            kCFPreferencesAnyHost );
    
    if (languages == NULL)
        return QLocale::system();

    CFStringRef uxstylelangs = CFStringCreateByCombiningStrings( kCFAllocatorDefault, languages, CFSTR( ":" ) );

    QString const s = CFStringToQString( uxstylelangs ).split( ':' ).value( 0 );
    return QLocale( s );
}
#endif
