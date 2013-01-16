/***************************************************************************
*   Copyright (C) 2005 - 2007 by                                          *
*      Jono Cole, Last.fm Ltd <jono@last.fm>                              *
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

#include "AbstractBootstrapper.h"

#include <lastfm/ws.h>
#include <lastfm/misc.h>

#include <QDebug>
#include <QNetworkRequest>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "zlib.h"

AbstractBootstrapper::AbstractBootstrapper( QObject* parent )
                     :QObject( parent )
{
}


AbstractBootstrapper::~AbstractBootstrapper(void)
{
}


bool
AbstractBootstrapper::zipFiles( const QString& inFileName, const QString& outFileName ) const
{
    QDir temp = QDir::temp();

    temp.remove( outFileName );

    gzFile outFile = gzopen( outFileName.toLocal8Bit(), "wb" );
    if ( !outFile )
        return false;

    QFile inFile( inFileName );
    if ( !inFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
        return false;

    if ( gzputs( outFile, inFile.readAll().data() ) < 1 )
        return false;

    gzclose( outFile );
    inFile.close();

    return true;
}


void
AbstractBootstrapper::sendZip( const QString& inFile )
{
    QString username = lastfm::ws::Username;
    QString timestamp = QString::number( QDateTime::currentDateTime().toUTC().toTime_t() );

    QMap<QString, QString> params;
    params["user"] = username;
    params["time"] = timestamp;
    params["auth"] = lastfm::md5( QString( QString( lastfm::ws::SharedSecret ) + timestamp ).toUtf8() );
    params["api_key"] = lastfm::ws::ApiKey;
    params["sk"] = lastfm::ws::SessionKey;

    //lastfm::ws::sign( params, true );
    //params["auth"] = params.take( "api_sig" );

    QUrl url( "http://bootstrap.last.fm/bootstrap/index.php" );

    QMapIterator<QString, QString> i( params );
    while ( i.hasNext() )
    {
        i.next();
        QByteArray const key = QUrl::toPercentEncoding( i.key() );
        QByteArray const value = QUrl::toPercentEncoding( i.value() );
        url.addEncodedQueryItem( key, value );
    }

    QFile* zipFile = new QFile( this );
    zipFile->setFileName( inFile );
    zipFile->open( QIODevice::ReadOnly );

    QNetworkRequest request( url );
    request.setRawHeader( "Content-type", "multipart/form-data, boundary=AaB03x" );
    request.setRawHeader( "Cache-Control", "no-cache" );
    request.setRawHeader( "Accept", "*/*" );

    QByteArray bytes;
    bytes.append( "--AaB03x\r\n" );
    bytes.append( "content-disposition: " );
    bytes.append( "form-data; name=\"agency\"\r\n" );
    bytes.append( "\r\n" );
    bytes.append( "0\r\n" );
    bytes.append( "--AaB03x\r\n" );
    bytes.append( "content-disposition: " );
    bytes.append( "form-data; name=\"bootstrap\"; filename=\"" + zipFile->fileName() + "\"\r\n" );
    bytes.append( "Content-Transfer-Encoding: binary\r\n" );
    bytes.append( "\r\n" );

    bytes.append( zipFile->readAll() );
    zipFile->close();

    bytes.append( "\r\n" );
    bytes.append( "--AaB03x--" );
    request.setHeader( QNetworkRequest::ContentLengthHeader, bytes.length() );

    qDebug() << "Sending " << url;

    emit percentageUploaded( 0 );

    QNetworkReply* reply = lastfm::nam()->post( request, bytes );
    connect( reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT( onUploadProgress(qint64,qint64)));
    connect( reply, SIGNAL(finished()), SLOT(onUploadDone()));
}


void
AbstractBootstrapper::onUploadProgress( qint64 done, qint64 total )
{
    emit percentageUploaded( int( float(done / total) * 100.0 ) );
}


void
AbstractBootstrapper::onUploadDone()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );

    qDebug() << reply->readAll();

    if( reply->error() == QNetworkReply::ContentAccessDenied ||
        reply->error() == QNetworkReply::ContentOperationNotPermittedError )
    {
        emit done( Bootstrap_Denied );
        return;
    }

    if( reply->error() != QNetworkReply::NoError )
    {
        qDebug() << reply->errorString();
        emit done( Bootstrap_UploadError );
        return;
    }

    qDebug() << "Bootstrap.zip sent to last.fm!";
    emit done( Bootstrap_Ok );
}
