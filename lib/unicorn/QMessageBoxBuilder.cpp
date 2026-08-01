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
#include "QMessageBoxBuilder.h"
#include <QApplication>

#include <QGridLayout>
#include <QLabel>

unicorn::MessageDialog::MessageDialog( QWidget* parent )
        :QDialog( parent ? ( parent->isVisible() ? parent : 0 ) : 0 ),
         m_clickedButton( QDialogButtonBox::NoButton )
{
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    setSizeGripEnabled( false );
    //setFixedWidth( 500 );

    setWindowFlags( Qt::Dialog );

    QGridLayout* l = new QGridLayout( this );

    l->addWidget( icon = new QLabel, 0, 0, 3, 1, Qt::AlignTop | Qt::AlignLeft );
    l->addWidget( label = new QLabel, 0, 1, 1, 1, Qt::AlignTop );
    label->setAlignment( Qt::AlignLeft | Qt::AlignTop );
    l->addWidget( informativeText = new QLabel, 1, 1, 1, 1, Qt::AlignTop );

    label->setWordWrap( true );
    informativeText->setWordWrap( true );
    informativeText->setAttribute( Qt::WA_MacSmallSize );

    QFont f = label->font();
    f.setBold( true ); 
    label->setFont( f );

    l->setColumnStretch( 1, 1 );
    l->setRowStretch( 3, 1 );

    l->addWidget( checkbox = new QCheckBox( tr( "Don't ask this again" )), 2, 1, 1, 1 );
    checkbox->setVisible( false );
    checkbox->setFocusPolicy( Qt::NoFocus );
    l->addWidget( buttons = new QDialogButtonBox, 3, 0, 1, 2 );
    buttons->setStandardButtons( QDialogButtonBox::Ok );
    connect( buttons, SIGNAL( clicked(QAbstractButton*)), SLOT( onButtonClicked(QAbstractButton*)));
    buttons->setFocus( Qt::ActiveWindowFocusReason );
}


void
unicorn::MessageDialog::onButtonClicked(QAbstractButton* b)
{
    m_clickedButton = int(buttons->standardButton( b ));
    switch( buttons->buttonRole( b )) {
        case QDialogButtonBox::AcceptRole:
        case QDialogButtonBox::YesRole:
            accept();
            break;

        case QDialogButtonBox::RejectRole:
        case QDialogButtonBox::NoRole:
            reject();
            break;
        default:
            break;
    }
}


QMessageBoxBuilder& 
QMessageBoxBuilder::setTitle( const QString& title )
{
#ifdef Q_OS_MAC
    box.setText( title + "\t\t\t" );
#else
    box.setWindowTitle( title );
#endif
    return *this;
}


QMessageBoxBuilder&
QMessageBoxBuilder::setText( const QString& text )
{
#ifdef Q_OS_MAC
    box.setInformativeText( text );
#else
    box.setText( text );
#endif
    return *this;
}


int
QMessageBoxBuilder::exec( bool* dontAskAgain )
{
    QApplication::setOverrideCursor( Qt::ArrowCursor );
    box.activateWindow();
    box.exec();
    QApplication::restoreOverrideCursor();
    
    if( dontAskAgain )
        *dontAskAgain = box.isDontShowAgainChecked();

    return box.clickedButton();
}
