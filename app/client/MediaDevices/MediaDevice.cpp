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
#include "MediaDevice.h"
#include "lib/unicorn/UnicornSettings.h"

#include <lastfm/misc.h>

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

MediaDevice::MediaDevice()
{
}

bool
MediaDevice::associateDevice( QString username )
{
    if ( deviceId().isEmpty() || deviceName().isEmpty() )
        return false;

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    if ( mountPath().isEmpty() || !QFile::exists( mountPath() ) )
        return false;
#endif

    unicorn::UserSettings us( username );
    int count = us.beginReadArray( "associatedDevices" );
    int index = count;

    QString devId;
    for ( int i = 0; i < count; i++ )
    {
        us.setArrayIndex( i );
        devId = us.value( "deviceId", "" ).toString();
        if ( devId == deviceId() )
        {
            index = i;
            break;
        }
    }

    us.endArray();

    us.beginWriteArray( "associatedDevices" );
    us.setArrayIndex( index );
    us.setValue( "deviceId", deviceId() );
    us.setValue( "deviceName", deviceName() );

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    us.setValue( "mountPath", QDir::toNativeSeparators( mountPath() ) );
#endif

    us.endArray();
    return true;
}

bool
MediaDevice::isDeviceKnown() const
{
    unicorn::UserSettings us;
    int count = us.beginReadArray( "associatedDevices" );
    QString devId;
    bool isKnown = false;
    for ( int i = 0; i < count; i++ )
    {
        us.setArrayIndex( i );
        devId = us.value( "deviceId", "" ).toString();
        if ( devId == deviceId() )
        {
            isKnown = true;
        }
    }
    us.endArray();
    return isKnown;
}

lastfm::User
MediaDevice::associatedUser( const QString deviceId )
{
    lastfm::User associatedUser( "" );

    // Check if the device has been associated with a user
    // and then if it is with the current user
    QList<lastfm::User> roster = unicorn::Settings().userRoster();
    foreach( lastfm::User user, roster )
    {
        unicorn::UserSettings us( user.name() );
        int count = us.beginReadArray( "associatedDevices" );

        for ( int i = 0; i < count; i++ )
        {
            us.setArrayIndex( i );

            QString tempDeviceId = us.value( "deviceId" ).toString();

            qDebug() << tempDeviceId;

            if ( tempDeviceId == deviceId )
            {
                associatedUser = user;
                break;
            }
        }
        us.endArray();

        if ( !associatedUser.name().isEmpty() ) break;
    }

    return associatedUser;
}
