/*
   Copyright (C) 2013 John Stamp <jstamp@mehercule.net>

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

#ifndef MEDIAPLAYER2_H
#define MEDIAPLAYER2_H

#include "DBusAbstractAdaptor.h"

class MediaPlayer2 : public DBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2");

    Q_PROPERTY( bool CanQuit READ CanQuit )
    Q_PROPERTY( bool CanRaise READ CanRaise )
    Q_PROPERTY( bool HasTrackList READ HasTrackList )
    Q_PROPERTY( QString Identity READ Identity )
    Q_PROPERTY( QString DesktopEntry READ DesktopEntry )
    Q_PROPERTY( QStringList SupportedUriSchemes READ SupportedUriSchemes )
    Q_PROPERTY( QStringList SupportedMimeTypes READ SupportedMimeTypes )

public:
    MediaPlayer2( QObject* parent );

    bool CanQuit() const;
    bool CanRaise() const;
    bool HasTrackList() const;
    QString Identity() const;
    QString DesktopEntry() const;
    QStringList SupportedUriSchemes() const;
    QStringList SupportedMimeTypes() const;

public slots:
    void Raise();
    void Quit();
};

#endif
