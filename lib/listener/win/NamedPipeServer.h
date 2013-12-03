/*
   Copyright 2012 Last.fm Ltd.
      - Primarily authored by Michael Coffey

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

#ifndef NAMEDPIPESERVER_H
#define NAMEDPIPESERVER_H

#include <QThread>

class NamedPipeServer : public QThread
{
    Q_OBJECT
public:
    explicit NamedPipeServer( QObject* parent = 0 );

private:
    void run();

signals:
    QString lineReady( const QString& line );
};

#endif // NAMEDPIPESERVER_H
