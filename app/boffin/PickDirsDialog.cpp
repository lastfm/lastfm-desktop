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
#include "PickDirsDialog.h"
#include <QtGui>
#include <QtWidgets>

#define kBlurb "Boffin creates Last.fm radio from the music on your computer."


PickDirsDialog::PickDirsDialog( QWidget* parent )
              : QDialog( parent, Qt::Sheet )
{
    ui.group = new QGroupBox( tr("Where is your music?") );
    ui.buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    ui.add = new QPushButton( tr("Add Another Folder") );
    ui.add->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Fixed );

    QLabel* blurb;

    QVBoxLayout* v = new QVBoxLayout( ui.group );
    v->addStretch();
    v->setMargin( 10 );

    v = new QVBoxLayout( this );
    v->addWidget( blurb = new QLabel( kBlurb ) );
    v->addSpacing( 18 );
    v->addWidget( ui.group );
    v->addSpacing( 8 );
    v->addWidget( ui.add );
    v->setAlignment( ui.add, Qt::AlignLeft );
    v->addSpacing( 25 );
    v->addWidget( ui.buttons );
    v->setSpacing( 0 );

    connect( ui.buttons, SIGNAL(rejected()), SLOT(reject()) );
    connect( ui.buttons, SIGNAL(accepted()), SLOT(accept()) );

    connect( ui.add, SIGNAL(clicked()), SLOT(prompt()) );
    
    ui.group->setMinimumHeight( 78 );

//////
    setMinimumWidth( 400 );

#ifdef Q_OS_MAC
    ui.group->setTitle( "" );
    QLabel* label;
    v->insertSpacing( 2, 6 );
    v->insertWidget( 2, label = new QLabel( "<b>Where is your music?") );
    v->insertSpacing( 0, 12 );
    label->setAttribute( Qt::WA_MacSmallSize );
#endif
}


void
PickDirsDialog::add( const QString& path )
{
    if (path.isEmpty()) return;

    QCheckBox* check = new QCheckBox( QDir::toNativeSeparators( path ) );
    check->setChecked( true );
    connect( check, SIGNAL(toggled( bool )), SLOT(enableDisableOk()) );
    
    QVBoxLayout* v = (QVBoxLayout*)ui.group->layout();
    v->insertWidget( v->count()-1, check );
    
    enableDisableOk();
}


void
PickDirsDialog::prompt()
{
    add( QFileDialog::getExistingDirectory( this ) );
}


QStringList
PickDirsDialog::dirs() const
{
    QStringList dirs;
    foreach (QCheckBox* check, ui.group->findChildren<QCheckBox*>())
        if (check->isChecked())
            dirs += check->text();
    dirs.removeAll( "" );
    return dirs;
}


void
PickDirsDialog::setDirs(QStringList dirs)
{
    qDebug() << dirs;
    
    if (dirs.isEmpty())
#ifdef __APPLE__
        dirs << QDir::home().filePath( "Music" );
#else
        dirs << QDir::homePath();
#endif
    
    foreach (QString dir, dirs)
        add( dir );
}


void
PickDirsDialog::enableDisableOk()
{
    QPushButton* ok = ui.buttons->button( QDialogButtonBox::Ok );
    ok->setEnabled( true );
    foreach (QCheckBox* check, ui.group->findChildren<QCheckBox*>())
        if (check->isChecked()) 
            return;
    ok->setEnabled( false );
}
