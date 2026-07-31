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
#ifndef MESSAGE_BOX_BUILDER_H
#define MESSAGE_BOX_BUILDER_H

#include <lib/DllExportMacro.h>
#include <QtWidgets/QMessageBox>
#include <QAbstractButton>
#include <QPushButton>

#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QCheckBox>
#include <QStyle>
#include <QDebug>
#include <QLabel>
#include <QDebug>

namespace unicorn {
    class UNICORN_DLLEXPORT MessageDialog : public QDialog {
        Q_OBJECT
    public:
        MessageDialog( QWidget* parent );
        void setStandardButtons( QMessageBox::StandardButtons b )
        { 
            buttons->setStandardButtons( QDialogButtonBox::StandardButtons(int(b) )); 
        }

        void setIcon( QMessageBox::Icon x )
        { 
            QIcon pm;
            switch( x ) {
                case QMessageBox::NoIcon:
                    break;
                case QMessageBox::Information:
                    pm = style()->standardIcon( QStyle::SP_MessageBoxInformation );
                    break;
                case QMessageBox::Warning:
                    pm = style()->standardIcon( QStyle::SP_MessageBoxWarning );
                    break;
                case QMessageBox::Critical:
                    pm = style()->standardIcon( QStyle::SP_MessageBoxCritical );
                    break;
                case QMessageBox::Question:
                    pm = style()->standardIcon( QStyle::SP_MessageBoxQuestion );
                    break;
            }
            icon->setPixmap( pm.pixmap( 32, 32 ) );
        }

        QAbstractButton* button( QMessageBox::StandardButton b )
        {
            QAbstractButton* ret = buttons->button( QDialogButtonBox::StandardButton(int(b) ));
            return ret;
        }

        void addButton( QAbstractButton* b, QMessageBox::ButtonRole r )
        {
            buttons->addButton( b, QDialogButtonBox::ButtonRole(int(r)));
        }

        void setText( const QString& t )
        {
            label->setText( t );
        }

        void setInformativeText( const QString& t )
        {
            informativeText->setText( t );
        }

        void setCheckBox( bool b )
        {
            checkbox->setVisible( b );
        }

        bool isDontShowAgainChecked() const
        {
            return checkbox->isChecked();
        }

        int clickedButton() const { return m_clickedButton; }

    private slots:
        void onButtonClicked(class QAbstractButton*);

    protected:
        QLabel *icon, *label, *informativeText;
        QDialogButtonBox* buttons;
        QCheckBox* checkbox;
        int m_clickedButton;
    };
}

class UNICORN_DLLEXPORT QMessageBoxBuilder
{
    unicorn::MessageDialog box;

public:
    /** Try not to use 0! */
    QMessageBoxBuilder( QWidget* parent ) : box( parent )
    {}
    
    QMessageBoxBuilder& setTitle( const QString& x );
    QMessageBoxBuilder& setText( const QString& x );
    /** the default is Information */
    QMessageBoxBuilder& setIcon( QMessageBox::Icon x ) { box.setIcon( x ); return *this; }
    /** the default is a single OK button */
    QMessageBoxBuilder& setButtons( QMessageBox::StandardButtons buttons ) { box.setStandardButtons( buttons ); return *this; }

    QMessageBoxBuilder& setButtonText( QMessageBox::StandardButton aButton, const QString& text )
    {
        box.button( aButton )->setText( text );
        return *this;
    }

    QMessageBoxBuilder& dontAskAgain(){ box.setCheckBox( true ); return *this; }

    QMessageBoxBuilder& addButton( QAbstractButton* b, QMessageBox::ButtonRole r ){ box.addButton( b, r ); return *this; }

    int exec( bool* dontAskAgain = 0 );
	
    QMessageBoxBuilder& sheet()
    {
#ifdef Q_OS_MAC
        if( box.parentWidget())
            box.setWindowFlags( Qt::Sheet | ( box.windowFlags() & ~Qt::Drawer ) );
#endif
        return *this;
    }
};

#endif // MESSAGE_BOX_BUILDER_H
