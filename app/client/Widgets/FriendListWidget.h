/*
   Copyright 2011 Last.fm Ltd.
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

#ifndef FRIENDLISTWIDGET_H
#define FRIENDLISTWIDGET_H

#include <QWidget>
#include <QPointer>

class QNetworkReply;

namespace unicorn { class Session; }
namespace lastfm { class XmlQuery; }
namespace lastfm { class User; }

namespace Ui { class FriendListWidget; }

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FriendListWidget( QWidget *parent = 0 );

public slots:
    void onCurrentChanged(int);

private slots:
    void onSessionChanged( const unicorn::Session& session );

    void onGotFriends();
    void onGotFriendsListeningNow();
    void onTextChanged( const QString& text );

    void onFindFriends();

    void refresh();

#ifdef Q_OS_MAC
    void scroll();
#endif

private:
    void showList();

private:
    QString m_currentUser;

    Ui::FriendListWidget* ui;
    QPointer<QMovie> m_movie;

    QPointer<QNetworkReply> m_reply;
};

#endif // FRIENDLISTWIDGET_H
