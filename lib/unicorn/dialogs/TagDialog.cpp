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

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QPushButton>

#include <lastfm/XmlQuery.h>
#include <lastfm/User.h>

#include "../widgets/SpinnerLabel.h"
#include "../widgets/TrackWidget.h"
#include "../widgets/UnicornTabWidget.h"
#include "../widgets/ItemSelectorWidget.h"
#include "../widgets/DataListWidget.h"
#include "../widgets/TagListWidget.h"
#include "../TrackImageFetcher.h"

#include "ui_TagDialog.h"
#include "TagDialog.h"

TagDialog::TagDialog( const Track& track, QWidget *parent )
        : unicorn::Dialog( parent, Qt::Tool ),
          ui( new Ui::TagDialog )
{
    ui->setupUi( this );

    m_track = track;

    ui->icon->setScaledContents( true );
    ui->icon->setHref( m_track.www() );

    ui->tags->setType( ItemSelectorWidget::Tag );

    layout()->setSizeConstraint( QLayout::SetFixedSize );
    setSizeGripEnabled( false );

    ui->title->setText( unicorn::Label::anchor( m_track.www().toString(), m_track.title() ) );

    if ( m_track.album().isNull() )
    {
        ui->description->setText( tr( "A track by %1" ).arg( unicorn::Label::anchor( m_track.artist().www().toString(), m_track.artist().name() ) ) );
        ui->album->setEnabled( false );
    }
    else
        ui->description->setText( tr( "A track by %1 from the release %2" ).arg( unicorn::Label::anchor( m_track.artist().www().toString(), m_track.artist().name() ), unicorn::Label::anchor( m_track.album().www().toString(), m_track.album() ) ) );

    m_imageFetcher = new TrackImageFetcher( m_track, Track::MediumImage );
    connect( m_imageFetcher, SIGNAL(finished(QPixmap)), ui->icon, SLOT(setPixmap(QPixmap)) );
    m_imageFetcher->startAlbum();
	
    ui->buttonBox->button( QDialogButtonBox::Ok )->setText( tr("Tag") );

    connect( ui->buttonBox, SIGNAL(accepted()), SLOT(onAccepted()) );
    connect( ui->buttonBox, SIGNAL(rejected()), SLOT(reject()) );

    connect( ui->tags, SIGNAL(changed()), SLOT(enableDisableOk()));
    enableDisableOk();

    QPushButton* dummyDefault = ui->buttonBox->addButton( QDialogButtonBox::Help );
    dummyDefault->setDefault( true );
    dummyDefault->setAutoDefault( true );
    dummyDefault->setVisible( false );

    ui->track->click();
}

void
TagDialog::enableDisableOk()
{
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( ui->tags->items().count() > 0 );
}

void
TagDialog::onAccepted()
{
    // call the ws for tagging
    QNetworkReply* reply;

    if ( ui->track->isChecked() )
        reply = m_track.addTags( ui->tags->items() );
    else if ( ui->album->isChecked() )
        reply = m_track.album().addTags( ui->tags->items() );
    else
        reply = m_track.artist().addTags( ui->tags->items() );

    connect( reply, SIGNAL(finished()), SLOT(onAddTagFinished()));

    setEnabled( false );
}

void
TagDialog::onAddTagFinished()
{
    XmlQuery lfm;

    if ( lfm.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {
        if ( lfm.attribute( "status" ) == "ok" )
            close();
        else
        {
            // TODO: display some kind of error message
            setEnabled( true );
        }
    }
    else
    {
        // TODO: display some kind of error message
        setEnabled( true );
    }
}
