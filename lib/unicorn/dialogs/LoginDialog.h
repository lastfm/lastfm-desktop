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
#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include "lib/DllExportMacro.h"
#include "lib/unicorn/UnicornSession.h"
#include <QDialog>
#include <QDialogButtonBox>


class UNICORN_DLLEXPORT LoginDialog : public QDialog
{
    Q_OBJECT
private:

    struct
    {
        class QLabel* title;
        class QLabel* description;
        class QDialogButtonBox* buttonBox;
    } ui;

public:
    LoginDialog( QWidget* parent = 0 );

private:
    QPushButton* ok() const { return ui.buttonBox->button( QDialogButtonBox::Ok ); }
};

#endif
