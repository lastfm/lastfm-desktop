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

#ifndef SHORTCUTEDIT_H
#define SHORTCUTEDIT_H

#include <QComboBox>

class ShortcutEdit : public QComboBox
{
    Q_OBJECT

public:
    ShortcutEdit( QWidget* parent = 0 );

    void setTextValue( QString str );
    void keyPressEvent( QKeyEvent* e );
    QString textValue() const;

    Qt::KeyboardModifiers modifiers() const;
    void setModifiers( Qt::KeyboardModifiers modifiers );
    int key() const;
    void setKey( int key );

private:
    Qt::KeyboardModifiers m_modifiers;
    int m_key;
};

#endif // SHORTCUTEDIT_H
