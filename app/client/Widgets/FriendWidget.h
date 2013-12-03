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

#ifndef FRIENDWIDGET_H
#define FRIENDWIDGET_H

#include <QFrame>
#include <QPointer>

#include <lastfm/XmlQuery.h>
#include <lastfm/User.h>
#include <lastfm/Track.h>

namespace Ui { class FriendWidget; }
namespace unicorn { class Label; }
using unicorn::Label;

class FriendWidget : public QFrame
{
    Q_OBJECT
public:
    explicit FriendWidget( const lastfm::XmlQuery& user, QWidget *parent = 0 );

    void update( const lastfm::XmlQuery& user, unsigned int order );
    void setOrder( int order );

    QString name() const;
    QString realname() const;

    void setListeningNow( bool listeningNow );

    bool operator<( const FriendWidget& that ) const;

    static QString genderString( const lastfm::Gender& gender );
    static QString userString( const lastfm::User& user );

private:
    void setDetails();

private slots:
    void updateTimestamp();

private:
    Ui::FriendWidget* ui;

    lastfm::User m_user;
    lastfm::MutableTrack m_track;
    unsigned int m_order;
    bool m_listeningNow;

    QPointer<QMovie> m_movie;
    QPointer<QTimer> m_timestampTimer;
};

#endif // FRIENDWIDGET_H
