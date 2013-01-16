
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMovie>
#include <QNetworkReply>
#include <QScrollBar>

#include <lastfm/User.h>
#include <lastfm/XmlQuery.h>
#include <lastfm/UrlBuilder.h>

#include "lib/unicorn/UnicornSession.h"
#include "lib/unicorn/DesktopServices.h"

#include "../Application.h"
#include "FriendWidget.h"
#include "FriendListWidget.h"
#include "RefreshButton.h"
#include "ui_FriendListWidget.h"


class FriendWidgetItem : public QListWidgetItem
{
public:
    FriendWidgetItem( QListWidget* parent );
    bool operator<( const QListWidgetItem& that ) const;
};

FriendWidgetItem::FriendWidgetItem( QListWidget* parent )
    :QListWidgetItem( parent )
{
}

bool
FriendWidgetItem::operator<( const QListWidgetItem& that ) const
{
    if ( !qobject_cast<FriendWidget*>(listWidget()->itemWidget( const_cast<QListWidgetItem*>(&that) )) )
        return false;

    if ( !qobject_cast<FriendWidget*>(listWidget()->itemWidget( const_cast<FriendWidgetItem*>(this) )) )
        return true;

    return *qobject_cast<FriendWidget*>(listWidget()->itemWidget( const_cast<FriendWidgetItem*>(this) )) <
            *qobject_cast<FriendWidget*>(listWidget()->itemWidget( const_cast<QListWidgetItem*>(&that) ));
}

FriendListWidget::FriendListWidget(QWidget *parent) :
    QWidget(parent),
    ui( new Ui::FriendListWidget )
{
    ui->setupUi( this );

#ifdef Q_OS_MAC
    connect( ui->friends->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(scroll()) );
#endif

    ui->noFriends->setText( tr( "<h3>You haven't made any friends on Last.fm yet.</h3>"
                                "<p>Find your Facebook friends and email contacts on Last.fm quickly and easily using the friend finder.</p>" ) );

    connect( ui->findFriends, SIGNAL(clicked()), SLOT(onFindFriends()) );

#if QT_VERSION >= 0x040700
    // The placeholder property was introduced in Qt 4.7
    ui->filter->setPlaceholderText( tr( "Search for a friend by username or real name" ) );
#endif
    ui->filter->setAttribute( Qt::WA_MacShowFocusRect, false );

    ui->friends->setObjectName( "friends" );
    ui->friends->setAttribute( Qt::WA_MacShowFocusRect, false );

    connect( ui->filter, SIGNAL(textChanged(QString)), SLOT(onTextChanged(QString)));

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );

    m_movie = new QMovie( ":/loading_meta.gif", "GIF", this );
    m_movie->setCacheMode( QMovie::CacheAll );
    ui->spinner->setMovie( m_movie );

    ui->stackedWidget->setCurrentWidget( ui->spinnerPage );
    m_movie->start();

    onSessionChanged( aApp->currentSession() );
}

#ifdef Q_OS_MAC
void
FriendListWidget::scroll()
{
    // KLUDGE: The friend list widgets don't move unless we do this
    ui->friends->sortItems( Qt::AscendingOrder );
}
#endif

void
FriendListWidget::onSessionChanged( const unicorn::Session& session )
{
    if ( session.user().name() != m_currentUser )
    {
        m_currentUser = session.user().name();

        if ( m_reply )
            m_reply->abort();

        ui->friends->clear();

        // add the refresh button
        FriendWidgetItem* item = new FriendWidgetItem( ui->friends );
        RefreshButton* refresh = new RefreshButton( this );
        refresh->setText( tr( "Refresh Friends" ) );
        ui->friends->setItemWidget( item, refresh );
        item->setSizeHint( refresh->sizeHint() );

        connect( refresh, SIGNAL(clicked()) , SLOT(refresh()));

        ui->stackedWidget->setCurrentWidget( ui->spinnerPage );
        m_movie->start();

        m_reply = session.user().getFriends( true, 50, 1 );
        connect( m_reply, SIGNAL(finished()), SLOT(onGotFriends()));
    }
}

void
FriendListWidget::onFindFriends()
{
    unicorn::DesktopServices::openUrl( lastfm::UrlBuilder( "findfriends" ).url() );
}

void
FriendListWidget::onTextChanged( const QString& text )
{
    QString trimmedText = text.trimmed();

    setUpdatesEnabled( false );

    if ( text.isEmpty() )
    {
        // special case an empty string so it's a bit zippier
        for ( int i = 1 ; i < ui->friends->count() ; ++i )
            ui->friends->item( i )->setHidden( false );
    }
    else
    {
        QRegExp re( QString( "^%1" ).arg( trimmedText ), Qt::CaseInsensitive );

        for ( int i = 1 ; i < ui->friends->count() ; ++i )
        {
            FriendWidget* user = static_cast<FriendWidget*>( ui->friends->itemWidget( ui->friends->item( i ) ) );

            ui->friends->item( i )->setHidden( !( user->name().startsWith( trimmedText, Qt::CaseInsensitive )
                                                       || user->realname().startsWith( trimmedText, Qt::CaseInsensitive )
                                                       || user->realname().split( ' ' ).filter( re ).count() > 0 ) );
        }
    }

    setUpdatesEnabled( true );
}


void FriendListWidget::onCurrentChanged( int index )
{
    if ( index == 3 )
        refresh();
}

void
FriendListWidget::refresh()
{
    if ( !m_reply
         || ( m_reply && m_reply->isFinished() ) )
    {
        RefreshButton* refresh = qobject_cast<RefreshButton*>(ui->friends->itemWidget( ui->friends->item( 0 ) ) );
        refresh->setEnabled( false );
        refresh->setText( tr( "Refreshing..." ) );

        m_reply = User().getFriendsListeningNow( 50, 1 );
        connect( m_reply, SIGNAL(finished()), SLOT(onGotFriendsListeningNow()));
    }
}

void
FriendListWidget::onGotFriends()
{
    // add this set of users to the list
    lastfm::XmlQuery lfm;
    if ( lfm.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {

        foreach( const lastfm::XmlQuery& user, lfm["friends"].children( "user" ) )
        {
            FriendWidgetItem* item = new FriendWidgetItem( ui->friends );
            FriendWidget* friendWidget = new FriendWidget( user, this );
            ui->friends->setItemWidget( item, friendWidget );
            item->setSizeHint( friendWidget->sizeHint() );
        }

        int page = lfm["friends"].attribute( "page" ).toInt();
        int perPage = lfm["friends"].attribute( "perPage" ).toInt();
        int totalPages = lfm["friends"].attribute( "totalPages" ).toInt();
        //int total = lfm["friends"].attribute( "total" ).toInt();

        // Check if we need to fetch another page of users
        if ( page != totalPages )
        {
            m_reply = lastfm::User().getFriends( true, perPage, page + 1 );
            connect( m_reply, SIGNAL(finished()), SLOT(onGotFriends()) );
        }
        else
        {
            // we have fetched all the pages!
            onTextChanged( ui->filter->text() );

            m_reply = User().getFriendsListeningNow( 50, 1 );
            connect( m_reply, SIGNAL(finished()), SLOT(onGotFriendsListeningNow()));
        }
    }
    else
    {
        showList();
    }
}


void
FriendListWidget::onGotFriendsListeningNow()
{
    // update the users in the list
    lastfm::XmlQuery lfm;
    if ( lfm.parse( qobject_cast<QNetworkReply*>(sender()) ) )
    {
        // reset all the friends to have the same order of max unsigned int
        for ( int i = 1 ; i < ui->friends->count() ; ++i )
            static_cast<FriendWidget*>( ui->friends->itemWidget( ui->friends->item( i ) ) )->setOrder( 0 - 1 );

        QList<XmlQuery> users = lfm["friendslisteningnow"].children( "user" );

        for ( int i = 0 ; i < users.count() ; ++i )
        {
            XmlQuery& user = users[i];

            for ( int j = 1 ; j < ui->friends->count() ; ++j )
            {
                FriendWidget* friendWidget = static_cast<FriendWidget*>( ui->friends->itemWidget( ui->friends->item( j ) ) );

                if ( friendWidget->name().compare( user["name"].text(),Qt::CaseInsensitive ) == 0 )
                    friendWidget->update( user, i );
            }

        }

        int page = lfm["friends"].attribute( "page" ).toInt();
        int perPage = lfm["friends"].attribute( "perPage" ).toInt();
        int totalPages = lfm["friends"].attribute( "totalPages" ).toInt();
        //int total = lfm["friends"].attribute( "total" ).toInt();

        // Check if we need to fetch another page of users
        if ( page != totalPages )
        {
            m_reply = lastfm::User().getFriends( true, perPage, page + 1 );
            connect( m_reply, SIGNAL(finished()), SLOT(onGotFriends()) );
        }
        else
        {
            // we have fetched all the pages!
            showList();
        }
    }
    else
    {
        // there was an error downloading a page
        showList();
    }

}

void
FriendListWidget::showList()
{
    onTextChanged( ui->filter->text() );

    ui->friends->sortItems( Qt::AscendingOrder );

    if ( ui->friends->count() == 1 )
        ui->stackedWidget->setCurrentWidget( ui->noFriendsPage );
    else
        ui->stackedWidget->setCurrentWidget( ui->friendsPage );

    m_movie->stop();

    RefreshButton* refresh = qobject_cast<RefreshButton*>(ui->friends->itemWidget( ui->friends->item( 0 ) ) );
    refresh->setEnabled( true );
    refresh->setText( tr( "Refresh Friends" ) );
}
