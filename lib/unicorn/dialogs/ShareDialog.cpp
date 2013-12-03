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

#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QEvent>
#include <QRadioButton>
#include <QComboBox>
#include <QListWidget>
#include <QDebug>
#include <QCheckBox>
#include <QDesktopWidget>

#include <lastfm/User.h>
#include <lastfm/XmlQuery.h>

#include "lib/unicorn/widgets/TrackWidget.h"
#include "lib/unicorn/DesktopServices.h"
#include "lib/unicorn/TrackImageFetcher.h"

#include "ShareDialog.h"

#include "ui_ShareDialog.h"

#include "../widgets/ItemSelectorWidget.h"

const int kMaxMessage(1000);

ShareDialog::ShareDialog( const Track& track, QWidget* parent )
    : unicorn::Dialog( parent, Qt::Tool ),
      ui( new Ui::ShareDialog ),
      m_track( track )
{
    ui->setupUi( this );

    ui->recipients->setType( ItemSelectorWidget::User );

    ui->icon->setScaledContents( true );
    ui->icon->setHref( m_track.www() );

    enableDisableOk();

    connect( ui->message, SIGNAL(textChanged()), SLOT(updateCharacterLimit()));
    connect( ui->message, SIGNAL(textChanged()), SLOT(enableDisableOk()));
    connect( ui->recipients, SIGNAL(changed()), SLOT(enableDisableOk()));

    ui->title->setText( unicorn::Label::anchor( m_track.www().toString(), m_track.title() ) );

    if ( m_track.album().isNull() )
        ui->description->setText( tr( "A track by %1" ).arg( unicorn::Label::anchor( m_track.artist().www().toString(), m_track.artist().name() ) ) );
    else
        ui->description->setText( tr( "A track by %1 from the release %2" ).arg( unicorn::Label::anchor( m_track.artist().www().toString(), m_track.artist().name() ), unicorn::Label::anchor( m_track.album().www().toString(), m_track.album() ) ) );

    m_imageFetcher = new TrackImageFetcher( m_track, Track::MediumImage );
    connect( m_imageFetcher, SIGNAL(finished(QPixmap)), ui->icon, SLOT(setPixmap(QPixmap)) );
    m_imageFetcher->startAlbum();

    QPushButton* dummyDefault = ui->buttons->addButton( QDialogButtonBox::Help );
    dummyDefault->setDefault( true );
    dummyDefault->setAutoDefault( true );
    dummyDefault->setVisible( false );
}

void
ShareDialog::enableDisableOk()
{
    ui->buttons->button( QDialogButtonBox::Ok )->setEnabled( ui->recipients->items().count() > 0
                      && ui->message->toPlainText().length() <= kMaxMessage );
}

void
ShareDialog::updateCharacterLimit()
{
    ui->characterLimit->setText( QString::number( ui->message->toPlainText().length() ) + "/" + QString::number(kMaxMessage) );

    if ( ui->message->toPlainText().length() > kMaxMessage ) {
        ui->characterLimit->setProperty( "xerror", true );
    }
    else
        ui->characterLimit->setProperty( "xerror", false );

    style()->polish( ui->characterLimit );
}

void
ShareDialog::onMessageChanged()
{
    // update the character message
    updateCharacterLimit();
}

void
ShareDialog::accept()
{
    QStringList recipients( ui->recipients->items() );
    QString const message = ui->message->toPlainText();
    bool isPublic = ui->isPublic->isChecked();

    // disable the dialog until we get a response from Last.fm
    setEnabled( false );
    connect( m_track.share( recipients, message, isPublic ), SIGNAL(finished()), SLOT(onShared()) );
}

void
ShareDialog::shareTwitter( const Track& track )
{
    QUrl twitterShareIntent( "http://twitter.com/intent/tweet" );
    twitterShareIntent.addEncodedQueryItem( "text", QUrl::toPercentEncoding( tr("Check out %1").arg( track.toString() ) ) );
    twitterShareIntent.addEncodedQueryItem( "url", QUrl::toPercentEncoding( track.www().toEncoded() ) );
    twitterShareIntent.addQueryItem( "via", "lastfm" );
    twitterShareIntent.addQueryItem( "related", "lastfm,lastfmpresents" );
    unicorn::DesktopServices::openUrl( twitterShareIntent );

}

void
ShareDialog::shareFacebook( const Track& track )
{
    QUrl facebookShareIntent( "http://www.facebook.com/sharer.php" );
    facebookShareIntent.addEncodedQueryItem( "t", QUrl::toPercentEncoding( track.toString() ) );
    facebookShareIntent.addEncodedQueryItem( "u", QUrl::toPercentEncoding( track.www().toEncoded() ) );
    unicorn::DesktopServices::openUrl( facebookShareIntent );
}

void
ShareDialog::onShared()
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

