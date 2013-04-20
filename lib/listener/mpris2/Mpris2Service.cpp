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

#include "Mpris2Service.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>

#define MPRIS2_PATH         "/org/mpris/MediaPlayer2"
#define MPRIS2_ROOT_IFACE   "org.mpris.MediaPlayer2"
#define MPRIS2_PLAYER_IFACE "org.mpris.MediaPlayer2.Player"
#define DBUS_PROPS_IFACE    "org.freedesktop.DBus.Properties"


static
QVariantMap demarshallMetadata(const QVariant &value)
{
    if ( !value.canConvert<QDBusArgument>() )
        return QVariantMap();

    QVariantMap metadata;
    QDBusArgument arg = value.value<QDBusArgument>();
    arg >> metadata;
    return metadata;
}


Mpris2Service::Mpris2Service( const QString& name, QObject * parent )
    : QObject( parent )
{
    m_state = "Stopped";

    m_propsIface = new QDBusInterface( name,
            MPRIS2_PATH,
            DBUS_PROPS_IFACE,
            QDBusConnection::sessionBus(),
            this );

    QDBusConnection::sessionBus().connect( name,
            MPRIS2_PATH,
            DBUS_PROPS_IFACE,
            "PropertiesChanged",
            this,
            SLOT( propsChanged( QString, QVariantMap, QStringList ) )
            );
}


Mpris2Service::~Mpris2Service()
{
}


QVariant
Mpris2Service::getProp( const QString& interface, const QString& prop ) const
{
    QDBusReply<QDBusVariant> reply = m_propsIface->call( "Get", interface, prop );
    if ( !reply.isValid() )
        return QVariant();

    return reply.value().variant();
}


QString
Mpris2Service::name() const
{
    return m_propsIface->service();
}


QString
Mpris2Service::identity() const
{
    return getProp( MPRIS2_ROOT_IFACE, "Identity" ).toString();
}


QString
Mpris2Service::desktopEntry() const
{
    return getProp( MPRIS2_ROOT_IFACE, "DesktopEntry" ).toString();
}


QString
Mpris2Service::artist() const
{
    QStringList entries = m_metadata.value( "xesam:artist" ).toStringList();
    if ( !entries.isEmpty() )
        return entries.first();
    return QString();
}


QString
Mpris2Service::title() const
{
    return m_metadata.value( "xesam:title" ).toString();
}


uint
Mpris2Service::length() const
{
    return m_metadata.value( "mpris:length" ).toLongLong() / 1000000;
}


QString
Mpris2Service::url() const
{
    return m_metadata.value( "xesam:url" ).toString();
}


void
Mpris2Service::propsChanged( const QString& str,
                                  const QVariantMap& changedProperties,
                                  const QStringList& invalidatedProperties )
{
    if ( changedProperties.contains( "Metadata" ) )
        m_metadata = demarshallMetadata( changedProperties.value( "Metadata" ) );
    else if ( invalidatedProperties.contains( "Metadata" ) )
        m_metadata = demarshallMetadata( getProp( MPRIS2_PLAYER_IFACE, "Metadata" ) );

    if ( changedProperties.contains( "PlaybackStatus" ) &&
         changedProperties.value( "PlaybackStatus" ).toString() != m_state )
    {
        m_state = changedProperties.value( "PlaybackStatus" ).toString();
        emit stateChanged( m_state );
    }
    else if ( invalidatedProperties.contains( "PlaybackStatus" ) )
    {
        m_state = getProp( MPRIS2_PLAYER_IFACE, "PlaybackStatus" ).toString();
    }
}
