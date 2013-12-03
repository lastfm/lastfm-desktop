/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Jono Cole and Michael Coffey

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

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

#include <lastfm/ScrobbleCache.h>

#include "../ScrobblesModel.h"

#include "ScrobbleConfirmationDialog.h"
#include "ui_ScrobbleConfirmationDialog.h"

ScrobbleConfirmationDialog::ScrobbleConfirmationDialog( const QList<lastfm::Track>& tracks, QWidget* parent )
    : QDialog( parent ), ui( new Ui::ScrobbleConfirmationDialog )
{
    ui->setupUi( this );

    m_toggled = true;

    m_scrobblesModel = new ScrobblesModel( this );
    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel( this );
    proxyModel->setSourceModel( m_scrobblesModel );
    ui->scrobblesView->setModel( proxyModel );

    ui->scrobblesView->sortByColumn( ScrobblesModel::TimeStamp, Qt::DescendingOrder );

    ui->scrobblesView->hideColumn( ScrobblesModel::Loved );
    ui->scrobblesView->hideColumn( ScrobblesModel::Album );

    connect( ui->toggleButton, SIGNAL( clicked() ), SLOT( toggleSelection() ) );

    addTracks( tracks );

    ui->invalidScrobbleWarning->setVisible( tracksToScrobble().count() != m_scrobblesModel->tracksToScrobble().count() );
}

ScrobbleConfirmationDialog::~ScrobbleConfirmationDialog()
{
    delete ui;
}

void
ScrobbleConfirmationDialog::setReadOnly()
{
    int count = 0;

    foreach ( const lastfm::Track& track, m_scrobblesModel->tracksToScrobble() )
        count += track.extra( "playCount" ).toInt();

    ui->infoText->setText( tr( "%n play(s) ha(s|ve) been scrobbled from a device", "", count ) );

    ui->buttons->removeButton( ui->buttons->button( QDialogButtonBox::No ) );
    ui->buttons->removeButton( ui->buttons->button( QDialogButtonBox::Yes ) );
    ui->buttons->addButton( QDialogButtonBox::Ok );

    ui->toggleButton->hide();
    ui->autoScrobble->hide();
    ui->invalidScrobbleWarning->hide();


    m_scrobblesModel->setReadOnly();
}

bool
ScrobbleConfirmationDialog::autoScrobble() const
{
    return ui->autoScrobble->isChecked();
}

void
ScrobbleConfirmationDialog::addTracks( const QList<lastfm::Track>& tracks )
{
    m_scrobblesModel->addTracks( tracks );

    // a hack to get the view to sort for the added tracks
    ui->scrobblesView->setSortingEnabled( false );
    ui->scrobblesView->setSortingEnabled( true );

    ui->scrobblesView->horizontalHeader()->setResizeMode( QHeaderView::Interactive );

    ui->scrobblesView->resizeColumnsToContents();
}

void
ScrobbleConfirmationDialog::addFiles( const QStringList& files )
{
    m_files << files;
}

const QStringList&
ScrobbleConfirmationDialog::files() const
{
    return m_files;
}

QList<lastfm::Track>
ScrobbleConfirmationDialog::tracksToScrobble() const
{
    QList<lastfm::Track> validTracks;

    foreach ( lastfm::Track track, m_scrobblesModel->tracksToScrobble() )
        if ( lastfm::ScrobbleCache::isValid( track ) )
            validTracks << track;

    return validTracks;
}

void
ScrobbleConfirmationDialog::toggleSelection()
{
    m_toggled = !m_toggled;

    for( int i = 0; i < m_scrobblesModel->rowCount(); i++ )
    {
        QModelIndex idx = m_scrobblesModel->index( i, ScrobblesModel::Artist, QModelIndex() );
        m_scrobblesModel->setData( idx, m_toggled, Qt::CheckStateRole );
    }
}
