/*
   Copyright 2005-2010 Last.fm Ltd.
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
#ifndef MEDIA_DEVICE_H
#define MEDIA_DEVICE_H

#include <QObject>
#include <QSqlDatabase>
#include <lastfm/Track.h>
#include <lastfm/User.h>

#define DB_NAME "MediaDevicesScrobbles"

class MediaDevice: public QObject
{
    Q_OBJECT

public:
    MediaDevice();

    /**
     * Associates the device to an user account.
     * @param username the user name to associate the device to. If no user name is provided the
     * device gets associated to the current user logged in.
     * @return true if it succeeds, false otherwise.
     */
    bool associateDevice( QString username = "" );

    /**
     * @return the last error ocurred or empty string if there wasn't any.
     */
    QString lastError() const { return m_error; }

    /**
     * @return an unique ID for the device.
     */
    virtual QString deviceId() const = 0;

    /**
     * @return the device name.
     */
    virtual QString deviceName() const = 0;


#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    /**
     * @return The mount path of the device.
     */
    virtual QString mountPath() const = 0;
#endif

    /**
     * @return true if the device is already associated with the user account, false otherwise.
     */
    bool isDeviceKnown() const;

    static lastfm::User associatedUser( const QString deviceId );

signals:
    void deviceScrobblingStarted();
    void calculatingScrobbles( int trackCount );
    void scrobblingCompleted( int trackCount );
    void errorOccurred();

protected:
    QString m_error;
};


#endif // MEDIA_DEVICE_H
