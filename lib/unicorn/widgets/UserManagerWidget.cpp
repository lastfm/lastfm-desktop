/*
   Copyright 2010-2012 Last.fm Ltd.
      - Primarily authored by Jono Cole, William Viana Soares and Michael Coffey

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

#include "UserManagerWidget.h"

#include "lib/unicorn/widgets/AvatarWidget.h"
#include "lib/unicorn/dialogs/LoginContinueDialog.h"
#include "lib/unicorn/dialogs/LoginDialog.h"
#include "lib/unicorn/LoginProcess.h"
#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/UnicornApplication.h"
#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/UnicornSession.h"

#include <lastfm/User.h>

#include <QApplication>
#include <QButtonGroup>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

using lastfm::User;
UserRadioButton::UserRadioButton( const User& user )
{
    addWidget( ui.button = new QRadioButton() );
    ui.button->setObjectName( "button" );

    addWidget( ui.image = new AvatarWidget() );
    ui.image->setObjectName( "image" );

    QVBoxLayout* userTextLayout = new QVBoxLayout();
    userTextLayout->setContentsMargins( 0, 0, 0, 0 );
    userTextLayout->setSpacing( 0 );

    userTextLayout->addWidget( ui.username = new QLabel( user.name() ) );
    userTextLayout->addWidget( ui.realName = new QLabel() );
    userTextLayout->addWidget( ui.loggedIn = new QLabel() );
    ui.realName->setObjectName( "realname" );
    addLayout(userTextLayout);

    addStretch();

    addWidget( ui.remove = new QPushButton( tr("Remove") ));

    setUser( user );

    if( user.imageUrl( User::MediumImage ).isEmpty() )
    {
        QNetworkReply* reply = User::getInfo( user.name() );
        connect( reply, SIGNAL(finished()), SLOT( onUserFetched()));
    }


    connect( ui.button, SIGNAL(clicked()), this, SIGNAL(clicked()) );
    connect( ui.remove, SIGNAL(clicked()), this, SIGNAL(remove()) );

    connect( qApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );
}

void
UserRadioButton::onSessionChanged( const unicorn::Session& session )
{
    if( ui.username->text() == session.user().name() )
        ui.loggedIn->setText( tr( "(currently logged in)" ) );
    else
        ui.loggedIn->clear();
}

void
UserRadioButton::click()
{
    ui.button->click();
}

bool
UserRadioButton::isChecked() const
{
    return ui.button->isChecked();
}

void
UserRadioButton::setUser( const lastfm::User& user )
{
    ui.username->setText( user.name() );

    if( !user.realName().isEmpty() )
        ui.realName->setText( QString( "(%1)" ).arg( user.realName() ) );

    if( user == User() )
        ui.loggedIn->setText( tr( "(currently logged in)" ) );

    if ( !user.imageUrl( User::MediumImage ).isEmpty() )
            ui.image->loadUrl( user.imageUrl( User::MediumImage ) );

    ui.image->setHref( user.www() );
}

void 
UserRadioButton::onUserFetched()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender());
    Q_ASSERT( reply );

    XmlQuery lfm;
    if ( lfm.parse( reply ) )
    {
        User user( lfm["user"] );
        setUser( user );
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}


const QString 
UserRadioButton::user() const
{
    return ui.username->text();
}


UserManagerWidget::UserManagerWidget( QWidget* parent )
    :QWidget( parent )
{
    m_lcd = new LoginContinueDialog( this );
    connect( m_lcd, SIGNAL( accepted()), SLOT( onLoginContinueDialogAccepted()));

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setSpacing( 10 );

    layout->addWidget( ui.groupBox = new QGroupBox() );

    ui.usersLayout = new QVBoxLayout( ui.groupBox );

    ui.groupBox->setTitle( tr( "Connected User Accounts:" ));

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    ui.addUserButton = new QPushButton( tr( "Add New User Account" ), this );

    buttonLayout->addWidget( ui.addUserButton );
    buttonLayout->addStretch( 1 );

    layout->addLayout( buttonLayout );

    layout->addStretch();

    QList<lastfm::User> roster = unicorn::Settings().userRoster();

    foreach( lastfm::User u, roster )
    {
        UserRadioButton* b = new UserRadioButton( u );
        add( b, false );
    }

    connect( ui.addUserButton, SIGNAL( clicked() ), SLOT( onAddUserClicked() ) );
}


UserManagerWidget::~UserManagerWidget()
{
}

void
UserManagerWidget::onAddUserClicked()
{
    LoginDialog* ld = new LoginDialog( this );
    ld->setWindowFlags( Qt::Sheet );
    ld->open();
    connect( ld, SIGNAL( accepted()), SLOT( onLoginDialogAccepted()) );
}


void
UserManagerWidget::onLoginDialogAccepted()
{
    LoginDialog* ld = qobject_cast<LoginDialog*>(sender());
    Q_ASSERT( ld );

    if ( m_loginProcess )
    {
        delete m_loginProcess;
        m_loginProcess = 0;
    }

    m_loginProcess = new unicorn::LoginProcess( this );

    ld->deleteLater();

    connect( qApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onLoginComplete(unicorn::Session)) );

    m_loginProcess->authenticate();

    m_lcd->setWindowFlags( Qt::Sheet );
    m_lcd->open();
}


void
UserManagerWidget::onLoginComplete( const unicorn::Session& session )
{
    Q_ASSERT( m_loginProcess );

    disconnect( qApp, SIGNAL(sessionChanged(unicorn::Session)), this, SLOT(onLoginComplete(unicorn::Session)) );

    if ( session.user().name().isEmpty() )
    {
        return;
    }

    bool alreadyAdded = false;
    foreach ( UserRadioButton* b, findChildren<UserRadioButton*>() )
    {
        if ( session.user().name() == b->user() )
        {
            alreadyAdded = true;
            break;
        }
    }

    if ( !alreadyAdded )
    {
        User user( session.user().name() );
        UserRadioButton* urb = new UserRadioButton( user );

        add( urb );
        if( ui.groupBox->layout()->count() <= 1 )
            urb->click();
    }
    else
    {
        QMessageBoxBuilder( this )
                    .setIcon( QMessageBox::Information )
                    .setTitle( tr( "Add User Error" ) )
                    .setText( tr( "This user has already been added." ) )
                    .exec();

    }
}

void 
UserManagerWidget::onLoginContinueDialogAccepted()
{
    m_loginProcess->getSession();
}

void
UserManagerWidget::onUserRemoved()
{
    UserRadioButton* urb = qobject_cast<UserRadioButton*>(sender());

    int result =
        QMessageBoxBuilder( parentWidget() ).setTitle( tr( "Removing %1" ).arg( urb->user() ) )
                                  .setText( tr( "Are you sure you want to remove this user? All user settings will be lost and you will need to re authenticate in order to scrobble in the future." ))
                                  .setIcon( QMessageBox::Question )
                                  .setButtons( QMessageBox::Yes | QMessageBox::No )
                                  .exec();

    if( result != QMessageBox::Yes )
        return;

    unicorn::Settings settings;
    settings.beginGroup( "Users" );
    settings.remove( urb->user() );
    settings.endGroup();

    if ( settings.userRoster().count() == 0 )
    {
        settings.setFirstRunWizardCompleted( false );
        qobject_cast<unicorn::Application*>( qApp )->restart();
    }

    // if this is the currently logged in user we will have to choose someone else
    if( urb->ui.button->isChecked() )
    {
        for ( int i = 0 ; i < ui.usersLayout->count() ; ++i )
        {
            UserRadioButton* button = qobject_cast<UserRadioButton*>(ui.usersLayout->itemAt( i )->layout());

            if ( button->user() != urb->user() )
            {
                // This is the first user in the list that are not deleting!

                button->click();

                unicorn::Settings s;
                s.setValue( "Username", button->user() );

                unicorn::UserSettings us( button->user() );
                QString sessionKey = us.value( "SessionKey", "" ).toString();
                qobject_cast<unicorn::Application *>( qApp )->changeSession( button->user(), sessionKey, true );

                break;
            }
        }
    }

    for ( int i = 0 ; i < layout()->count() ; ++i )
    {
        QLayoutItem* item = layout()->itemAt( i );

        if ( qobject_cast<UserRadioButton*>( item->widget() ) == urb )
        {
            delete layout()->takeAt( i );
            break;
        }
    }

    urb->deleteLater();
}

void 
UserManagerWidget::add( UserRadioButton* urb, bool announce )
{
    ui.usersLayout->addLayout( urb );

    if ( urb->user() == User().name() )
        urb->click();

    connect( urb, SIGNAL(remove()), SLOT(onUserRemoved()));
    connect( urb, SIGNAL( clicked() ), this, SIGNAL( userChanged() ) );

    if ( announce )
        emit rosterUpdated();

    connect( urb, SIGNAL( destroyed(QObject*)), SIGNAL( rosterUpdated()));
}


UserRadioButton*
UserManagerWidget::checkedButton() const
{
    for ( int i = 0 ; i < ui.usersLayout->count() ; ++i )
    {
        UserRadioButton* button = qobject_cast<UserRadioButton*>(ui.usersLayout->itemAt( i )->layout());

        if ( button && button->isChecked() )
        {
            return button;
        }
    }

    return 0;
}
