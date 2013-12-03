/*
   Copyright 2013 Last.fm Ltd.
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

#include "CheckFileSystemModel.h"

CheckFileSystemModel::CheckFileSystemModel( QObject *parent )
    :QFileSystemModel( parent )
{
}

Qt::ItemFlags
CheckFileSystemModel::flags( const QModelIndex& index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant
CheckFileSystemModel::data( const QModelIndex& index, int role) const
{
    if (role == Qt::CheckStateRole)
    {
        qint64 id = index.internalId();
        return m_checkTable.contains(id) ? m_checkTable.value(id) : Qt::Unchecked;
    }
    else
    {
        return QFileSystemModel::data(index, role);
    }
}

bool
CheckFileSystemModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if (role == Qt::CheckStateRole)
    {
        m_checkTable.insert(index.internalId(), (Qt::CheckState)value.toInt());

        emit dataChanged(index, index);
        emit dataChangedByUser(index);

        return true;
    }
    else
    {
        return QFileSystemModel::setData(index, value, role);
    }
}

void
CheckFileSystemModel::setCheck( const QModelIndex& index, const QVariant& value)
{
    m_checkTable.insert(index.internalId(), (Qt::CheckState)value.toInt());
    emit dataChanged(index, index);
}

Qt::CheckState
CheckFileSystemModel::getCheck( const QModelIndex& index )
{
    return (Qt::CheckState)data(index, Qt::CheckStateRole).toInt();
}
