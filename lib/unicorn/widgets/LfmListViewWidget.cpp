/*
   Copyright 2005-2009 Last.fm Ltd.
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

#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>

#include "LfmListViewWidget.h"
#include <lastfm/User.h>
#include <lastfm/Artist.h>
#include <lastfm/Track.h>

#include <iostream>

LfmDelegate::LfmDelegate( QAbstractItemView* parent ):QStyledItemDelegate(parent)
{
    m_viewSize = parent->size();
    parent->installEventFilter( this );
}

void LfmDelegate::paint( QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index ) const
{
    QIcon icon;

    if( index.data(Qt::DecorationRole).type() == QVariant::Icon )
    {
        icon = index.data(Qt::DecorationRole).value<QIcon>();

        if ( icon.isNull() )
            //icon = QIcon( m_defaultImage );
            icon = QIcon( ":/default_user.png" );

        QRect iconRect = opt.rect.translated( 3, 3 );
        iconRect.setSize( QSize( 34, 34 ));
        icon.paint( p, iconRect );
        QSize iconSize = icon.actualSize( iconRect.size());

        if( iconSize.isEmpty()) iconSize = QSize( 34, 34 );

        iconRect.translate( ( iconRect.width() - iconSize.width()) / 2.0f,
                            ( iconRect.height() - iconSize.height()) /2.0f );
        iconRect.setSize( iconSize );
        p->drawRect( iconRect );
    }

    QFontMetrics fm( p->font() );
    QString elidedText = fm.elidedText( index.data().toString(), Qt::ElideRight, opt.rect.width() - 50 );
    p->drawText( opt.rect.adjusted( 46, 3, -5, -5 ), elidedText );
}

QSize LfmDelegate::sizeHint( const QStyleOptionViewItem& opt, const QModelIndex& /*index*/ ) const
{
    QFontMetrics fm( opt.font );
    //int textWidth = fm.width( index.data().toString());
    int spacing = qobject_cast<QListView*>(parent())->spacing();
    return QSize( (m_viewSize.width() / 2)-(spacing*2), 40 );
}

bool LfmDelegate::eventFilter( QObject* obj, QEvent* event )
{
    if( event->type() == QEvent::Resize ) {
        QWidget* view = qobject_cast< QWidget* >(obj );

        if( !view ) return false;

        m_viewSize = view->size();
        emit sizeHintChanged( QModelIndex() );
    }
    return false;
}

QPixmap LfmDelegate::defaultImage() const
{
    return m_defaultImage;
}

void LfmDelegate::setDefaultImage( QPixmap defaultImage )
{
    m_defaultImage = defaultImage;
}


void
LfmItem::onImageLoaded()
{
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    QPixmap px;
    px.loadFromData( reply->readAll() );
    m_icon = QIcon( px );
    emit updated();
}


void LfmItem::loadImage( const QUrl& url )
{
    QString imageUrl = url.toString();
   
    QNetworkReply* reply = lastfm::nam()->get(QNetworkRequest( url ));
    connect( reply, SIGNAL( finished()), this, SLOT( onImageLoaded()));
}


void
LfmListModel::addUser( const User& a_user )
{
    User* user = new User;
    *user = a_user;
    LfmItem* item = new LfmItem( user );
    item->loadImage( user->imageUrl(User::SmallImage, true ));

    beginInsertRows( QModelIndex(), rowCount(), rowCount());
    m_items << item;
    connect( item, SIGNAL(updated()), SLOT( itemUpdated()));
    endInsertRows();
}


void 
LfmListModel::addArtist( const Artist& a_artist )
{
    Artist* artist = new Artist;
    *artist = a_artist;
    LfmItem* item = new LfmItem( artist );
    item->loadImage( artist->imageUrl( Artist::SmallImage, true ));

    beginInsertRows( QModelIndex(), rowCount(), rowCount());
    m_items << item;
    connect( item, SIGNAL(updated()), SLOT( itemUpdated()));
    endInsertRows();   
}

void
LfmListModel::itemUpdated()
{
    LfmItem* item = static_cast<LfmItem*>(sender());
    int index = m_items.indexOf( item );
    if ( index >= 0 )
        emit dataChanged( createIndex( index, 0), createIndex( index, 0));
}


QVariant 
LfmListModel::data( const QModelIndex & index, int role ) const
{
    if( index.row() > m_items.count()) return QVariant();

    const LfmItem& item = *(m_items[index.row()]);

    switch( role ) {
        case Qt::DisplayRole:
            return item.m_type->toString();

        case Qt::DecorationRole:
            return item.m_icon;

        case Qt::ToolTipRole:
            return item.m_type->toString();

        case CursorRole:
            return static_cast<int>( Qt::PointingHandCursor );

        case WwwRole:
            return item.m_type->www();

    }
    return QVariant();
}



LfmListView::LfmListView(QWidget *parent):
   QListView(parent),
   m_lastRow(-1)
{
   setMouseTracking(true);
   setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
}


#include <QMouseEvent>
void LfmListView::mouseMoveEvent(QMouseEvent *event)
{
   QAbstractItemModel *m(model());

   if (m)
   {
      QModelIndex index = indexAt(event->pos());
      if (index.isValid())
      {
         // When the index is valid, compare it to the last row.
         // Only do something when the the mouse has moved to a new row.
         if (index.row() != m_lastRow)
         {
            m_lastRow = index.row();
            // Request the data for the CursorRole.
            QVariant data = m->data(index, LfmListModel::CursorRole );

            Qt::CursorShape shape = Qt::ArrowCursor;
            if (!data.isNull())
               shape = static_cast<Qt::CursorShape>(data.toInt());
            setCursor(shape);
         }
      }
      else
      {
         if (m_lastRow != -1)
            setCursor(Qt::ArrowCursor);
         m_lastRow = -1;
      }
   }
   QListView::mouseMoveEvent(event);
}
