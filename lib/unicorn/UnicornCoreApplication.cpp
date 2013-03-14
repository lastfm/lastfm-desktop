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
#include <lastfm/Fingerprint.h>

#include "UnicornCoreApplication.h"

#include "common/c++/Logger.h"

using namespace lastfm;

#ifdef WIN32
extern void qWinMsgHandler( QtMsgType t, const char* msg );
#endif

unicorn::CoreApplication::CoreApplication( const QString& id, int& argc, char** argv )
                      : QtSingleCoreApplication( id, argc, argv )
{
    init();
}

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

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#ifdef Q_OS_MAC
    QString pluginsDir = applicationDirPath() + "/../plugins";
#else
    QString pluginsDir = applicationDirPath() + "/plugins";
#endif
    addLibraryPath( pluginsDir );
#endif


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
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    qDebug() << "Plugin DIR" << pluginsDir;
#endif
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

bool
unicorn::CoreApplication::notify(QObject* receiver, QEvent* event )
{
    try
    {
        return QCoreApplication::notify( receiver, event );
    }
    catch( const lastfm::Fingerprint::Error& e )
    {
        qDebug() << "Fingerprint error" << e;
        qApp->quit();
    }
    catch(...)
    {
       qDebug() << "Exception caught.";
       qApp->quit();
    }
}
