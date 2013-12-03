/*
   Copyright 2013 Last.fm Ltd.
      - Primarily authored by Michael Coffey

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Fingerprinter.h"

#include "lib/unicorn/UnicornCoreApplication.h"

#include <lastfm/ws.h>

#include <QStringList>
#include <QCoreApplication>
#include <QDebug>

// ./fingerprinter --username <username> --filename <filename> --title <title> --album <album> --artist <artist>

int main(int argc, char *argv[])
{
    int exitCode = -1;

    QtSingleCoreApplication::setApplicationName( "Last.fm Fingerprinter" );
    QtSingleCoreApplication::setOrganizationName( "Last.fm" );

    unicorn::CoreApplication a(argc, argv);

    qDebug() << a.arguments();

    int usernameIndex = a.arguments().indexOf( "--username" );
    int filenameIndex = a.arguments().indexOf( "--filename" );

    if ( usernameIndex != -1 && filenameIndex != -1 )
    {
        // username and filename are required fields
        lastfm::ws::Username = a.arguments().at( usernameIndex + 1 );

        // create the track from the command line arguments
        MutableTrack track;
        track.setUrl( QUrl::fromLocalFile( a.arguments().at( filenameIndex + 1 ) ) );

        int titleIndex = a.arguments().indexOf( "--title" );
        int albumIndex = a.arguments().indexOf( "--album" );
        int artistIndex = a.arguments().indexOf( "--artist" );

        if ( titleIndex != -1 ) track.setTitle( a.arguments().at( titleIndex + 1 ) );
        if ( albumIndex != -1 ) track.setAlbum( a.arguments().at( albumIndex + 1 ) );
        if ( artistIndex != -1 ) track.setTitle( a.arguments().at( artistIndex + 1 ) );

        Fingerprinter* fingerprinter = new Fingerprinter( track );
        exitCode = a.exec();
        delete fingerprinter;
    }
    else
    {
        qWarning() << "Usage: fingerprinter --username <username> --filename <filename> --title <title> --album <album> --artist <artist>";
    }

    return exitCode;
}
