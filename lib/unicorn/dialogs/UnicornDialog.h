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

#ifndef UNICORN_DIALOG_H_
#define UNICORN_DIALOG_H_

#include <QDialog>
#include <QApplication>
#include <QDesktopWidget>
#include "../UnicornMainWindow.h"
#include "../UnicornSettings.h"
#include <math.h>

#include "lib/DllExportMacro.h"

namespace unicorn {
class UNICORN_DLLEXPORT Dialog : public QDialog
{
Q_OBJECT
public:
    Dialog( QWidget* parent = 0, Qt::WindowFlags f = 0 )
    :QDialog( parent, f ), m_firstShow( true )
    {
        connect( qApp->desktop(), SIGNAL( resized(int)), SLOT( cleverlyPosition()));
    }

    virtual void setVisible( bool visible )
    {
        if( !visible || !m_firstShow ) return QDialog::setVisible( visible );
        m_firstShow = false;

        cleverlyPosition();
        return QDialog::setVisible( visible );
    }

protected:
    void saveState( const QObject* object, const QString& key, const QVariant& value ) const
    {
        AppSettings s;
        s.beginGroup( QString( metaObject()->className()));
        s.beginGroup( QString( object->metaObject()->className()));
        s.setValue( key, value );
    }

    QVariant restoreState( const QObject* object, const QString& key, const QVariant& defaultValue = QVariant() ) const
    {
        AppSettings s;
        s.beginGroup( QString( metaObject()->className()));
        s.beginGroup( QString( object->metaObject()->className()));
        return s.value( key, defaultValue );
    }

    virtual void moveEvent( QMoveEvent* event )
    {
        using unicorn::MainWindow;
        QWidget* mw = findMainWindow();
        if( !mw ) return QDialog::moveEvent( event );

        AppSettings s;
        s.beginGroup( QString( metaObject()->className() ));
            s.setValue( "position", (pos() - mw->pos()));
        return QDialog::moveEvent( event );
    }

private slots:
    void cleverlyPosition() 
    {
        //Clever positioning
        QWidget* mw = findMainWindow();

        if( !mw ) return;

        resize( sizeHint() );

        AppSettings s;

        s.beginGroup( QString( metaObject()->className() ));
        QPoint offset = s.value( "position", QPoint( 40, 40 )).toPoint();
        s.endGroup();

        move( mw->pos() + offset);

        int screenNum = qApp->desktop()->screenNumber( mw );
        QRect screenRect = qApp->desktop()->availableGeometry( screenNum );
        if( !screenRect.contains( frameGeometry(), true)) {
            QRect diff;

            diff = screenRect.intersected( frameGeometry() );

            if (diff.left() == screenRect.left() )
                move( diff.left(), pos().y());
            if( diff.right() == screenRect.right())
                move( diff.right() - width(), pos().y());
            if( diff.top() == screenRect.top())
                move( pos().x(), diff.top());
            if( diff.bottom() == screenRect.bottom())
                move( pos().x(), diff.bottom() - height());
        }
    }

private:
    bool m_firstShow;
    QWidget* findMainWindow() const
    {
        unicorn::MainWindow* mw = 0;
        foreach( QWidget* w, qApp->topLevelWidgets() ) {
            mw = qobject_cast<unicorn::MainWindow*>( w );
            if( mw ) break;
        }
        return mw;
    };
};

} //unicorn

#endif //UNICORN_DIALOG_H_
