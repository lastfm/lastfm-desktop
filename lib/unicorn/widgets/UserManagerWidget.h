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

#ifndef USER_MANAGER_WIDGET_H_
#define USER_MANAGER_WIDGET_H_

#include <QLayout>
#include <QRadioButton>
#include <QPointer>

#include "lib/DllExportMacro.h"

namespace lastfm{ class User; }
namespace unicorn
{
    class LoginProcess;
    class Session;
}

class LoginContinueDialog;

class QAbstractButton;
class QButtonGroup;
class QFrame;
class QLabel;
class QPushButton;
class QVBoxLayout;

class UNICORN_DLLEXPORT UserRadioButton : public QHBoxLayout
{
Q_OBJECT

    friend class UserManagerWidget;

public:
    UserRadioButton( const lastfm::User& user );
    const QString user() const;

    void click();
    bool isChecked() const;

signals:
    void clicked();
    void remove();

private:
    void setUser( const lastfm::User& user );

private slots:
    void onUserFetched();
    void onSessionChanged( const unicorn::Session& session );

private:
    struct
    {
        QRadioButton* button;
        QPushButton* remove;
        QLabel* username;
        QLabel* realName;
        QLabel* loggedIn;
        class AvatarWidget* image;
        QFrame* frame;
    } ui;
};


class UNICORN_DLLEXPORT UserManagerWidget : public QWidget
{
Q_OBJECT
public:
    UserManagerWidget( QWidget* parent = 0 );
    ~UserManagerWidget();

    UserRadioButton* checkedButton() const;

signals:
    void rosterUpdated();
    void userChanged();

protected slots:
    void onAddUserClicked();
    void onLoginDialogAccepted();
    void onLoginContinueDialogAccepted();
    void onUserRemoved();

protected:
    void add( UserRadioButton*, bool = true );

    struct {
        class QGroupBox* groupBox;
        class QVBoxLayout* usersLayout;
        class QPushButton* addUserButton;
    } ui;


private slots:
    void onLoginComplete( const unicorn::Session& session );

private:
    QPointer<unicorn::LoginProcess> m_loginProcess;
    QPointer<LoginContinueDialog> m_lcd;
};

#endif //USER_MANAGER_WIDGET_H_
