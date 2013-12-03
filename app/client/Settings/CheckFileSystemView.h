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

#ifndef CHECKFILESYSTEMVIEW_H
#define CHECKFILESYSTEMVIEW_H

#include "CheckFileSystemModel.h"

#include <QTreeView>

class CheckFileSystemView : public QTreeView
{
    Q_OBJECT
public:
    explicit CheckFileSystemView( QWidget* parent = 0 );

    void checkPath( const QString& path, Qt::CheckState state );
    void setExclusions( const QStringList& list );
    QStringList getExclusions();

signals:
    void dataChanged();

private:
    void fillDown( const QModelIndex& index);
    void updateParent( const QModelIndex& index );
    void getExclusionsForNode( const QModelIndex& index, QStringList&exclusions);

private slots:
    void onCollapse( const QModelIndex& idx );
    void onExpand( const QModelIndex& idx);
    void updateNode( const QModelIndex& idx);

private:
    CheckFileSystemModel* m_fsModel;
    QSet<qint64>  m_expandedSet;
};

#endif // CHECKFILESYSTEMVIEW_H
