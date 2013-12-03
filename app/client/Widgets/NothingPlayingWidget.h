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

#ifndef NOTHINGPLAYINGWIDGET_H
#define NOTHINGPLAYINGWIDGET_H

#include <QFrame>

namespace lastfm { class User; }
namespace unicorn { class Session; }

namespace Ui { class NothingPlayingWidget; }

class NothingPlayingWidget : public QFrame
{
    Q_OBJECT
public:
    explicit NothingPlayingWidget( QWidget* parent = 0 );

private:
    void setUser( const lastfm::User& user );
#ifdef Q_OS_WIN
    void startApp( const QString& app );
#endif

private slots:
    void onSessionChanged( const unicorn::Session& session );

#if defined( Q_OS_MAC ) || defined( Q_OS_WIN )
    void oniTunesClicked();
#endif
#ifdef Q_OS_WIN
    void onWinampClicked();
    void onWMPClicked();
    void onFoobarClicked();
#endif

private:
    Ui::NothingPlayingWidget* ui;
};

#endif // NOTHINGPLAYINGWIDGET_H
