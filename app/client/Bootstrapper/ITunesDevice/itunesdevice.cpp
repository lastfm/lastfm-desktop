/***************************************************************************
 *   Copyright 2005 - 2008 Last.fm Ltd.                                    *
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

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include "itunesdevice.h"
#include "ITunesParser.h"

#ifdef WIN32
    #include "windows.h"
    #include "shlobj.h"
#endif


QString
ITunesDevice::LibraryPath()
{
    if ( !m_iTunesLibraryPath.isEmpty() )
        return m_iTunesLibraryPath;

    QString path;
    QString confPath;

#ifdef Q_OS_MAC
    QSettings ist( "apple.com", "iTunes" );
    path = ist.value( "AppleNavServices:ChooseObject:0:Path" ).toString();
    path = path.remove( "file://localhost" );
    qDebug() << "Found iTunes Library in:" << path;

    QFileInfo fi( path + "iTunes Music Library.xml" );
    if ( fi.exists() )
        m_iTunesLibraryPath = fi.absoluteFilePath();
    else
        m_iTunesLibraryPath = QFileInfo( QDir::homePath() + "/Music/iTunes/iTunes Music Library.xml" ).absoluteFilePath();

    return m_iTunesLibraryPath;
#endif

#ifdef WIN32
    {
        // Get path to My Music
        char acPath[MAX_PATH];
        HRESULT h = SHGetFolderPathA( NULL, CSIDL_MYMUSIC,
                                      NULL, 0, acPath );

        if ( h == S_OK )
            path = QString::fromLocal8Bit( acPath );
//        else
//            LOG( 1, "Couldn't get My Music path\n" );

        qDebug() << "CSIDL_MYMUSIC path: " << path;
    }

    {
        // Get path to Local App Data
        char acPath[MAX_PATH];
        HRESULT h = SHGetFolderPathA( NULL, CSIDL_LOCAL_APPDATA,
                                      NULL, 0, acPath );

        if ( h == S_OK )
            confPath = QString::fromLocal8Bit( acPath );
//        else
//            LOG( 1, "Couldn't get Local Application Data path\n" );

        qDebug() << "CSIDL_LOCAL_APPDATA path: " << confPath;
    }

    // Try reading iTunesPrefs.xml for custom library path
    QFile f( confPath + "/Apple Computer/iTunes/iTunesPrefs.xml" );
    if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        qDebug() << "Found iTunesPrefs.xml";

        QByteArray content = f.readAll();

        int tagStart = content.indexOf( "iTunes Library XML Location:1" );

        if ( tagStart != -1 )
        {
            // TODO: this could fail if the XML is broken
            int dataTagStart = content.indexOf( "<data>", tagStart );
            int dataTagEnd = dataTagStart + 6;
            int dataEndTagStart = content.indexOf( "</data>", dataTagStart );
            QByteArray lp = content.mid( dataTagEnd, dataEndTagStart - dataTagEnd );

            qDebug() << "lp before trim: " << lp;

            // The file contains whitespace and linebreaks in the middle of
            // the data so need to squeeze all that out
            lp = lp.simplified();
            lp = lp.replace( ' ', "" );

            qDebug() << "lp after simplified: " << lp;

            lp = QByteArray::fromBase64( lp );

            qDebug() << "lp after base64: " << lp;

            QString sp = QString::fromUtf16( (ushort*)lp.data() );

            qDebug() << "Found iTunes Library path (after conversion to QString):" << sp;

            QFileInfo fi( sp );
            if ( fi.exists() )
            {
                qDebug() << "file exists, returning: " << fi.absoluteFilePath();
                m_iTunesLibraryPath = fi.absoluteFilePath();
                return m_iTunesLibraryPath;
            }
        }
        else
        {
            qDebug() << "No custom library location found in iTunesPrefs.xml";
        }
    }

    // Fall back to default path otherwise
    m_iTunesLibraryPath = path + "/iTunes/iTunes Music Library.xml";

    qDebug() << "Will use default iTunes Library path: " << m_iTunesLibraryPath;

    return m_iTunesLibraryPath;

#endif

    // Fallback for testing
//    m_iTunesLibraryPath = "/tmp/iTunes Music Library.xml";
//    return m_iTunesLibraryPath;
}

ITunesDevice::ITunesDevice() :
      m_totalSize( 0 ),
      m_file( 0 ),
      m_handler( 0 ),
      m_xmlReader( 0 ),
      m_xmlInput( 0 )
{

}


Track
ITunesDevice::firstTrack( const QString& file )
{
    m_database = file;

    if ( !m_file )
    {
        m_file = new QFile( file );
        if ( !m_file->open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            qDebug() << "Could not open iTunes Library" << m_database;
            return Track();
        }

        m_totalSize = m_file->size();
        m_xmlReader = new QXmlSimpleReader();
        m_xmlInput = new QXmlInputSource();

        m_handler = new ITunesParser();
        m_xmlReader->setContentHandler( m_handler );

        m_xmlInput->setData( m_file->read( 32768 ) );
        if ( !m_xmlReader->parse( m_xmlInput, true ) )
        {
            qDebug() << "Couldn't read file: " << m_database;
            return Track();
        }
    }

    return nextTrack();
}


Track
ITunesDevice::nextTrack()
{
    while ( m_handler->trackCount() < 20 && !m_file->atEnd() )
    {
        m_xmlInput->setData( m_file->read( 32768 ) );
        m_xmlReader->parseContinue();

        emit progress( (float)( (float)m_file->pos() / (float)m_file->size() ) * 100.0, m_handler->peekTrack() );
    }

    Track t = m_handler->takeTrack();
    if ( !t.isNull() )
    {
        return t;
    }

    if ( m_file->atEnd() )
    {
        // Finished with the database, let's close our stuff
        qDebug() << "Finished reading";

        m_file->close();
        delete m_file;
        m_file = 0;
    }

    return Track();
}
