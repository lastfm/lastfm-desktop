/***************************************************************************
 *   Copyright (C) 2005 - 2008 by                                          *
 *      Last.fm Ltd <client@last.fm>                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/


#include "ITunesPluginInstaller.h"

#include "lib/unicorn/dialogs/CloseAppsDialog.h"
#include "lib/unicorn/QMessageBoxBuilder.h"

#include <CoreFoundation/CoreFoundation.h>

#include <QApplication>
#include <QDir>
#include <QProcess>
#include <QDebug>

static const char* kBundleName = "AudioScrobbler.bundle";
static const char* kPListFile = "Contents/Info.plist";

QString
shippedPluginDir()
{
    return qApp->applicationDirPath() + "/" + kBundleName + "/";
}

QString
iTunesPluginDir()
{
    return QDir::homePath() + "/Library/iTunes/iTunes Plug-ins/" + kBundleName + "/";
}

unicorn::ITunesPluginInstaller::ITunesPluginInstaller( QWidget* parent )
    :QObject( parent ),
      m_needsTwiddlyBootstrap( false )
{
    qDebug() << " shippedPluginDir: " << shippedPluginDir();
}

unicorn::Version
unicorn::ITunesPluginInstaller::installedVersion()
{
    return pListVersion( iTunesPluginDir() + kPListFile );
}

unicorn::Version
unicorn::ITunesPluginInstaller::bundledVersion()
{
    return pListVersion( shippedPluginDir() + kPListFile );
}

void
unicorn::ITunesPluginInstaller::install()
{
    qDebug() << "Found installed iTunes plugin?" << isPluginInstalled();

    Version installVersion;
    Version shippedVersion;

    disableLegacyHelperApp();

    if ( isPluginInstalled() ) 
    {
        installVersion = installedVersion();
        qDebug() << "Found installed iTunes plugin version:" << installVersion.toString();
    }
    else
        //TODO don't bootstrap if installation fails
        m_needsTwiddlyBootstrap = true;

    shippedVersion = bundledVersion();

    if ( shippedVersion == Version() )
    {
        qDebug() << "Could not locate shipped iTunes plugin!" << shippedPluginDir();
    }
    else
    {
        qDebug() << "Found shipped iTunes plugin version:" << shippedVersion.toString();

        if ( installVersion != shippedVersion )
        {
            bool closeAppsShown = false;

            qDebug() << "Installing shipped iTunes plugin...";

            QWidget* dialogParent = 0;
            if ( parent()->isWidgetType() )
                dialogParent = qobject_cast<QWidget*>( parent() );

            unicorn::CloseAppsDialog* closeApps = new unicorn::CloseAppsDialog( dialogParent );

            closeApps->setTitle( tr( "Close iTunes for plugin update!" ) );
            closeApps->setDescription( tr( "<p>Your iTunes plugin (%2) is different to the one shipped with this version of the app (%1).</p>"
                                           "<p>Please close iTunes now to update.</p>" ).arg( shippedVersion.toString(), installVersion == Version() ? tr( "not installed" ) : installVersion.toString() ) );
            closeApps->showPluginList( false );

            if ( closeApps->result() != QDialog::Accepted )
            {
                closeAppsShown = true;
                closeApps->exec();
            }
            else
                closeApps->deleteLater();

            if ( closeApps->result() == QDialog::Accepted )
            {
                if ( !removeInstalledPlugin() )
                {
                    qDebug() << "Removing installed plugin from" << iTunesPluginDir() << "failed!";

                    // The user didn't close their media players
                    QMessageBoxBuilder( dialogParent ).setTitle( tr( "Your plugin hasn't been installed" ) )
                            .setIcon( QMessageBox::Warning )
                            .setText( tr( "There was an error while removing the old plugin" ) )
                            .setButtons( QMessageBox::Ok )
                            .exec();
                }
                else
                {
                    qDebug() << "Successfully removed installed plugin.";

                    if ( installPlugin() )
                    {
                        qDebug() << "Successfully installed the plugin.";

                        if ( closeAppsShown )
                        {
                            // Tell the user that
                            QMessageBoxBuilder( dialogParent ).setTitle( tr( "iTunes Plugin installed!" ) )
                                    .setIcon( QMessageBox::Information )
                                    .setText( tr( "<p>Your iTunes plugin has been installed.</p>"
                                                  "<p>You're now ready to device scrobble.</p>" ) )
                                    .setButtons( QMessageBox::Ok )
                                    .exec();
                        }
                    }
                    else
                    {
                        qDebug() << "Installing the plugin failed!";

                        // The user didn't close their media players
                        QMessageBoxBuilder( dialogParent ).setTitle( tr( "Your plugin hasn't been installed" ) )
                                .setIcon( QMessageBox::Warning )
                                .setText( tr( "There was an error while copying the new plugin into place" ) )
                                .setButtons( QMessageBox::Ok )
                                .exec();
                    }
                }
            }
            else
            {
                qDebug() << "Plugin not installed. User reject the dialog.";

                // The user didn't close their media players
                QMessageBoxBuilder( dialogParent ).setTitle( tr( "Your plugin hasn't been installed" ) )
                        .setIcon( QMessageBox::Warning )
                        .setText( tr( "You didn't close iTunes" ) )
                        .setButtons( QMessageBox::Ok )
                        .exec();
            }
        }
        else
            qDebug() << "Installed iTunes plugin is up-to-date.";
    }
}

bool
unicorn::ITunesPluginInstaller::isPluginInstalled()
{
    return QDir( iTunesPluginDir() ).exists();
}


unicorn::Version
unicorn::ITunesPluginInstaller::pListVersion( const QString& file )
{
    QFile f( file );
    if ( !f.open( QIODevice::ReadOnly ) )
        return Version();

    const QString key( "<key>CFBundleVersion</key>" );
    const QString begin( "<string>" );
    const QString end( "</string>" );

    while ( !f.atEnd() )
    {
        QString line = f.readLine();
        if ( line.contains( key ) && !f.atEnd() )
        {
            QString versionLine = QString( ( f.readLine() ) ).trimmed();
            QString versionString = versionLine.section( begin, 1 );
            versionString = versionString.left( versionString.length() - end.length() );
            Version version = Version::fromString( versionString );

            f.close();
            return version;
        }
    }

    f.close();
    return Version();
}


static bool
deleteDir( QString path )
{
    if ( !path.endsWith( "/" ) )
        path += "/";

    QDir d( path );

    // Remove all files
    const QStringList files = d.entryList( QDir::Files );
    foreach( QString file, files )
    {
        if ( !d.remove( file ) )
            return false;
    }

    // Remove all dirs (recursive)
    const QStringList dirs = d.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
    foreach( QString dir, dirs )
    {
        if ( !deleteDir( path + dir ) )
            return false;
    }

    return d.rmdir( path );
}


bool
unicorn::ITunesPluginInstaller::removeInstalledPlugin()
{
    if ( !isPluginInstalled() )
        return true;

    return deleteDir( iTunesPluginDir() );
}


static bool
copyDir( QString path, QString dest )
{
    if ( !path.endsWith( '/' ) )
        path += '/';
    if ( !dest.endsWith( '/' ) )
        dest += '/';

    QDir( dest ).mkpath( "." );
    QDir d( path );

    const QStringList files = d.entryList( QDir::Files );
    foreach( QString file, files )
    {
        QFile f( path + file );
        if ( !f.copy( dest + file ) )
            return false;
    }

    const QStringList dirs = d.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
    foreach( QString dir, dirs )
    {
        if ( !copyDir( path + dir, dest + dir ) )
            return false;
    }

    return true;
}


bool
unicorn::ITunesPluginInstaller::installPlugin()
{
    return copyDir( shippedPluginDir(), iTunesPluginDir() );
}


static inline QByteArray
CFStringToUtf8( CFStringRef s )
{
    QByteArray result;

    if (s != NULL) 
    {
        CFIndex length;
        length = CFStringGetLength( s );
        length = CFStringGetMaximumSizeForEncoding( length, kCFStringEncodingUTF8 ) + 1;
        char* buffer = new char[length];

        if (CFStringGetCString( s, buffer, length, kCFStringEncodingUTF8 ))
            result = QByteArray( buffer );
        else
            qWarning() << "CFString conversion failed.";

        delete[] buffer;
    }

    return result;
}



void
unicorn::ITunesPluginInstaller::disableLegacyHelperApp()
{
    qDebug() << "Disabling old LastFmHelper";

    // EVEN MORE LEGACY: disable oldest helper auto-launch!
    QString oldplist = QDir::homePath() + "/Library/LaunchAgents/fm.last.lastfmhelper.plist";
    if ( QFile::exists( oldplist ) )
    {
        QProcess::execute( "/bin/launchctl", QStringList() << "unload" << oldplist );
        QFile::remove( oldplist );
    }

    // REMOVE LastFmHelper from loginwindow.plist
    CFArrayRef prefCFArrayRef = (CFArrayRef)CFPreferencesCopyValue( CFSTR( "AutoLaunchedApplicationDictionary" ), 
                                                                    CFSTR( "loginwindow" ), 
                                                                    kCFPreferencesCurrentUser, 
                                                                    kCFPreferencesAnyHost );
    if ( prefCFArrayRef == NULL ) return;
    CFMutableArrayRef tCFMutableArrayRef = CFArrayCreateMutableCopy( NULL, 0, prefCFArrayRef );
    if ( tCFMutableArrayRef == NULL ) return;

    for ( int i = CFArrayGetCount( prefCFArrayRef ) - 1; i >= 0 ; i-- )
    {
        CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex( prefCFArrayRef, i );
        QString path = QString::fromUtf8( CFStringToUtf8( (CFStringRef)CFDictionaryGetValue( dict, CFSTR( "Path" ) ) ) );

        if ( path.toLower().contains( "lastfmhelper" ) )
        {
            // Better make sure LastFmHelper is really dead
            QProcess::execute( "/usr/bin/killall", QStringList() << "LastFmHelper" );

            qDebug() << "Removing helper from LoginItems at position" << i;
            CFArrayRemoveValueAtIndex( tCFMutableArrayRef, (CFIndex)i );
        }
    }

    CFPreferencesSetValue( CFSTR( "AutoLaunchedApplicationDictionary" ), 
                           tCFMutableArrayRef, 
                           CFSTR( "loginwindow" ), 
                           kCFPreferencesCurrentUser, 
                           kCFPreferencesAnyHost );
    CFPreferencesSynchronize( CFSTR( "loginwindow" ), 
                              kCFPreferencesCurrentUser, 
                              kCFPreferencesAnyHost );

    CFRelease( prefCFArrayRef );
    CFRelease( tCFMutableArrayRef );
}


void
unicorn::ITunesPluginInstaller::uninstall()
{
    QDir d = iTunesPluginDir() + "Contents/Resources";
    
    // always use absolute paths to tools! - muesli
    QProcess::startDetached( "/bin/sh", 
                             QStringList() << d.filePath( "uninstall.sh" ),
                             d.path() );
}
