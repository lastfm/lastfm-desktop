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
#ifndef IPOD_DEVICE_H
#define IPOD_DEVICE_H

#include "MediaDevice.h"

class IpodDevice: public MediaDevice
{
    Q_OBJECT
public:
    // DO NOT CHANGE THIS: IT WILL MESS UP IPOD SCROBBLE SETTINGS
    IpodDevice( const QString& deviceId, const QString& deviceName);

    virtual QString deviceId() const;
    virtual QString deviceName() const;

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    /**
     * @return The mount path of the device.
     */
    virtual QString mountPath() const{ return QString(); }
#endif

private:
    void setSetting( QString key, QVariant value );
    QVariant setting( QString key, QVariant defaultValue );

private:
    QString m_deviceId;
    QString m_deviceName;
};

#endif // IPOD_DEVICE_H
