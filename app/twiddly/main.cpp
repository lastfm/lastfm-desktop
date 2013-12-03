/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole, Erik Jaelevik, 
        Christian Muehlhaeuser

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

#include "IPod.h"
#include "TwiddlyApplication.h"
#include "Utils.h"

#include "plugins/iTunes/ITunesExceptions.h"
#include "common/c++/Logger.h"
#include "lib/unicorn/UnicornCoreApplication.h"

#ifdef Q_OS_MAC
#include "lib/unicorn/plugins/Version.h"
#include "lib/unicorn/plugins/ITunesPluginInstaller.h"
#else
#include "lib/unicorn/plugins/ITunesPluginInfo.h"
#endif

#include <lastfm/misc.h>

#include <QtCore>
#include <QtXml>
#include <iostream>

void writeXml( const QDomDocument&, const QString& path );
void logException( QString );


/**
  *
  * Fake params: --device ipod --connection usb --pid 4611 --vid 1452 --serial 000000C8B035
  */
int
main( int argc, char** argv )
{
    // Make sure the logger exists in this binary
#ifdef Q_OS_WIN
    QString bytes = TwiddlyApplication::log( TwiddlyApplication::applicationName() ).absoluteFilePath();
    const wchar_t* path = bytes.utf16();
#else
    QByteArray bytes = TwiddlyApplication::log( TwiddlyApplication::applicationName() ).absoluteFilePath().toLocal8Bit();
    const char* path = bytes.data();
#endif
    new Logger( path );

    TwiddlyApplication::setApplicationName( "iPodScrobbler" );
    TwiddlyApplication::setApplicationVersion( "2" );

    TwiddlyApplication app( argc, argv );

    if ( app.isRunning() )
    {
        qDebug() << "The iPod Scrobbler is already running. Shutting down" << app.arguments();
        return 1;
    }

    // check we're using a compatible version of the plugin
    unicorn::Version compatibleVersion( 6, 0, 5, 4 );
    unicorn::Version installedVersion;
#ifdef Q_OS_WIN
    unicorn::ITunesPluginInfo* iTunesPluginInfo = new unicorn::ITunesPluginInfo;
    installedVersion = iTunesPluginInfo->installedVersion();
    delete iTunesPluginInfo;
#else
    // TODO: get the actual installed version
    installedVersion = unicorn::ITunesPluginInstaller::installedVersion();
#endif

    if ( installedVersion < compatibleVersion )
    {
        // tell the app that the plugin is incompatible
        QStringList args;
        args << "--tray";
        args << "--twiddly";
        args << "incompatible-plugin";

        Utils::startAudioscrobbler( args );

        qDebug() << "The iTunes pluggin is old and incompatible. Please update. Shutting down" << app.arguments();
        return 1;
    }

    try
    {
        if ( app.arguments().contains( "--bootstrap-needed?" ) )
        {
            return AutomaticIPod::PlayCountsDatabase().isBootstrapNeeded();
        }        

        QDateTime start_time = QDateTime::currentDateTime();
        QTime time;
        time.start();

        if ( app.arguments().contains( "--bootstrap" ) )
        {
            AutomaticIPod::PlayCountsDatabase().bootstrap();
        }
        else // twiddle!
        {
            app.sendBusMessage( "--twiddling" );

            IPod* ipod = IPod::fromCommandLineArguments( app.arguments() );

            {
                QStringList args;
                args << "--tray";
                args << "--twiddly";
                args << "starting";
                args << "--deviceId";
                args << ipod->uid();
                args << "--deviceName";
                args << ipod->name;

                Utils::startAudioscrobbler( args );
            }

            qDebug() << "Twiddling device: " << ipod->serial;
            ipod->twiddle();

            //------------------------------------------------------------------
            IPodType previousType = ipod->settings().type();
            IPodType currentType = app.arguments().contains( "--manual" ) ? IPodManualType : IPodAutomaticType;

            if ( previousType == IPodManualType && currentType == IPodAutomaticType )
            {
                qDebug() << "iPod switched from manual to automatic - deleting manual db and ignoring scrobbles";

                // The iPod was manual, but is now automatic, we must:
                // 1. remove the manual db, to avoid misscrobbles if it ever becomes
                //    manual again
                QString path = ManualIPod::PlayCountsDatabase( ipod ).path();
                QFile( path ).remove();

                // 2. not scrobble this time :( because any tracks that were on the
                //    the iPod and are also in the iTunes library will be merged
                //    and if they ever played on the iPod, will increase the iTunes
                //    library playcounts. We need to sync the Automatic playcountsdb
                //    but not scrobble anything.
                ipod->scrobbles().clear();
            }

            ipod->settings().setType( currentType );
            //------------------------------------------------------------------

            if (ipod->scrobbles().count())
            {
                // create a unique storage location for the XML
                QDir dir = ipod->saveDir().filePath( "scrobbles" );
                QString filename = QDateTime::currentDateTime().toString( "yyyyMMddhhmmss" ) + ".xml";
                QString path = dir.filePath( filename );
                dir.mkpath( "." );

                QDomDocument xml = ipod->scrobbles().xml();
                xml.documentElement().setAttribute( "uid", ipod->uid() );
                writeXml( xml, path );

                QStringList args;
                args << "--tray";
                args << "--twiddly";
                args << "complete";
                args << "--ipod-path";
                args << path;
                args << "--deviceId";
                args << ipod->uid();
                args << "--deviceName";
                args << ipod->name;

                Utils::startAudioscrobbler( args );
            }
            else
            {
                QStringList args;
                args << "--tray";
                args << "--twiddly";
                args << "no-tracks-found";
                args << "--deviceId";
                args << ipod->uid();
                args << "--deviceName";
                args << ipod->name;

                Utils::startAudioscrobbler( args );
            }

            // do last so we don't record a sync if we threw and thus it "didn't" happen
            ipod->settings().setLastSync( start_time );
            delete ipod;           
        }
        
        qDebug() << "Procedure took:" << time.elapsed() << "milliseconds";
    }
    catch( QString& s )
    {
        logException( s );
    }
    catch( std::string& s )
    {
        logException( QString::fromStdString( s ) );
    }
    catch( const char* s )
    {
        logException( s );
    }
    catch( ITunesException& e )
    {
        logException( e.what() );
    }
    
    return 0;
}


void
writeXml( const QDomDocument& xml, const QString& path )
{
    // we write to a temporary file, and then do an atomic move
    // this prevents the client from potentially reading a corrupt XML file
    QTemporaryFile f;

    f.setAutoRemove( false );
    
    if (!f.open())
        throw "Couldn't write XML";

    QTextStream s( &f );
    xml.save( s, 2 );
    
    if ( !f.rename( path ) )
        throw QString("Couldn't move to ") + path;
}


void
logException( QString message )
{
    qCritical() << "FATAL ERROR:" << message;
    
    // we do this because LfmApp splits on spaces in parseMessage()
    message.replace( ' ', '_' );
    static_cast<TwiddlyApplication*>(qApp)->sendBusMessage( QString( "container://Notification/Twiddly/Error/" + message ).toAscii() );
}
