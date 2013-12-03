/**************************************************************************
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

#ifndef ABSTRACTBOOTSTRAPPER_H
#define ABSTRACTBOOTSTRAPPER_H
#include <QObject>

class QString;

/**
  * @author Jono Cole <jono@last.fm>
  *
  * @brief The AbstractBootstrapper class is the abstract base class of
  *        bootstrapper implementations, providing common functionality.
  *
  * Subclasses of this class can implement the bootStrap method to perform
  * the bootstrapping operation and then call the zipFile / sendZip methods
  * to submit the bootstrap.
  */
class AbstractBootstrapper : public QObject
{
    Q_OBJECT

public:

    enum BootstrapStatus
    {
        Bootstrap_Ok = 0,
        Bootstrap_UploadError,
        Bootstrap_Denied,
        Bootstrap_Spam, /* eg. 1 billion plays for Bjork */
        Bootstrap_Cancelled
    };

    AbstractBootstrapper( QObject* parent = NULL );
    virtual ~AbstractBootstrapper(void);

    bool zipFiles( const QString& inFileName, const QString& outFileName ) const;
    void sendZip( const QString& inFile );
    virtual void bootStrap() = 0;

signals:
    void percentageUploaded( int );
    void done( AbstractBootstrapper::BootstrapStatus status );

protected slots:
    void onUploadDone();
    void onUploadProgress( qint64 done, qint64 total );
};

#endif //ABSTRACTBOOTSTRAPPER_H

