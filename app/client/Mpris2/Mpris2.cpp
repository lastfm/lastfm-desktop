/***********************************************************************
 * Copyright 2012  Eike Hein <hein@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#include "Mpris2.h"

#include "MediaPlayer2.h"
#include "MediaPlayer2Player.h"

#include <unistd.h>

#include <QDBusConnection>

Mpris2::Mpris2( QObject *parent )
    : QObject( parent )
{
    QString mpris2Name( "org.mpris.MediaPlayer2.lastfm-scrobbler" );

    bool success = QDBusConnection::sessionBus().registerService( mpris2Name );

    // If the above failed, it's likely because we're not the first instance
    // and the name is already taken. In that event the MPRIS2 spec wants the
    // following:
    if (!success) {
        mpris2Name = mpris2Name + ".instance" + QString::number( getpid() );
        success = QDBusConnection::sessionBus().registerService( mpris2Name );
    }

    if ( success )
    {
        DBusAbstractAdaptor *adaptor = new MediaPlayer2( this );
        adaptor->setDBusPath( "/org/mpris/MediaPlayer2" );
        adaptor = new MediaPlayer2Player( this );
        adaptor->setDBusPath( "/org/mpris/MediaPlayer2" );
        QDBusConnection::sessionBus().registerObject( "/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAdaptors );
    }
}

Mpris2::~Mpris2()
{
}

