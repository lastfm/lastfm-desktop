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

#include "CheckFileSystemView.h"

#include <QDesktopServices>

CheckFileSystemView::CheckFileSystemView( QWidget* parent )
    :QTreeView( parent )
{
    m_fsModel = new CheckFileSystemModel( this );
    m_fsModel->setRootPath( "" );
    m_fsModel->setFilter( QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks );
    setModel( m_fsModel );

    setRootIndex( m_fsModel->index( "" ) );

    setColumnHidden(1, true);
    setColumnHidden(2, true);
    setColumnHidden(3, true);
    //header()->hide();

    connect( m_fsModel, SIGNAL(dataChangedByUser(const QModelIndex&)), SLOT(updateNode(const QModelIndex&)));
    connect( m_fsModel, SIGNAL(dataChangedByUser(const QModelIndex&)), SIGNAL(dataChanged()));
    connect( this, SIGNAL(collapsed(const QModelIndex&)), SLOT(onCollapse(const QModelIndex&)));
    connect( this, SIGNAL(expanded(const QModelIndex&)), SLOT(onExpand(const QModelIndex&)));
}


void
CheckFileSystemView::checkPath( const QString& path, Qt::CheckState state )
{
    QModelIndex index = m_fsModel->index(path);
    m_fsModel->setCheck(index, state);
    updateNode(index);
}

void
CheckFileSystemView::setExclusions( const QStringList& list )
{
    foreach(QString path, list)
    {
        checkPath(path, Qt::Checked);
    }
}

QStringList
CheckFileSystemView::getExclusions()
{
    QStringList exclusions;
    QModelIndex root = rootIndex();

    getExclusionsForNode(root, exclusions);

    return exclusions;
}

void
CheckFileSystemView::getExclusionsForNode( const QModelIndex& index, QStringList& exclusions)
{
    // Look at first node
    // Is it unchecked?
    //  - move on to next node
    // Is it checked?
    //  - add to list
    //  - move to next node
    // Is it partially checked?
    //  - recurse

    int numChildren = m_fsModel->rowCount(index);
    for (int i = 0; i < numChildren; ++i)
    {
        QModelIndex kid = m_fsModel->index(i, 0, index);
        Qt::CheckState check = m_fsModel->getCheck(kid);
        if (check == Qt::Unchecked)
        {
            continue;
        }
        else if (check == Qt::Checked)
        {
            exclusions.append(m_fsModel->filePath(kid));
        }
        else if (check == Qt::PartiallyChecked)
        {
            getExclusionsForNode(kid, exclusions);
        }
        else
        {
            Q_ASSERT(false);
        }
    }
}

void
CheckFileSystemView::onCollapse( const QModelIndex& /*idx*/ )
{

}

void
CheckFileSystemView::onExpand( const QModelIndex& idx )
{
    // If the node is partially checked, that means we have been below it
    // setting some stuff, so only fill down if we are unchecked.
    if (m_fsModel->getCheck(idx) != Qt::PartiallyChecked)
    {
        fillDown(idx);
    }
}

void
CheckFileSystemView::updateNode( const QModelIndex& idx )
{
    // Start by recursing down to the bottom and then work upwards
    fillDown(idx);
    updateParent(idx);
}

void
CheckFileSystemView::fillDown( const QModelIndex& parent )
{
    // Recursion stops when we reach a directory which has never been expanded
    // or one that has no children.
    if (!isExpanded(parent) ||
        !m_fsModel->hasChildren(parent))
    {
        return;
    }

    Qt::CheckState state = m_fsModel->getCheck(parent);
    int numChildren = m_fsModel->rowCount(parent);
    for (int i = 0; i < numChildren; ++i)
    {
        QModelIndex kid = m_fsModel->index(i, 0, parent);
        m_fsModel->setCheck(kid, state);
        fillDown(kid);
    }
}

void
CheckFileSystemView::updateParent( const QModelIndex& index )
{
    QModelIndex parent = index.parent();

    if (!parent.isValid())
    {
        // We have reached the root
        return;
    }

    // Initialise overall state to state of first child
    QModelIndex kid = m_fsModel->index(0, 0, parent);
    Qt::CheckState overall = m_fsModel->getCheck(kid);

    int numChildren = m_fsModel->rowCount(parent);
    for (int i = 1; i <= numChildren; ++i)
    {
        kid = m_fsModel->index(i, 0, parent);
        Qt::CheckState state = m_fsModel->getCheck(kid);
        if (state != overall)
        {
            // If we ever come across a state different than the first child,
            // we are partially checked
            overall = Qt::PartiallyChecked;
            break;
        }
    }

    m_fsModel->setCheck(parent, overall);

    updateParent(parent);
}
