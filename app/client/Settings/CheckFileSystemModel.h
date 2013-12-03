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

#ifndef CHECKFILESYSTEMMODEL_H
#define CHECKFILESYSTEMMODEL_H

#include <QFileSystemModel>

class CheckFileSystemModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit CheckFileSystemModel( QObject *parent = 0 );
    
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    void setCheck( const QModelIndex& index, const QVariant& value);
    Qt::CheckState getCheck( const QModelIndex& index );

signals:
    void dataChangedByUser( const QModelIndex & index);

private:
    QHash<qint64, Qt::CheckState> m_checkTable;
};

#endif // CHECKFILESYSTEMMODEL_H
