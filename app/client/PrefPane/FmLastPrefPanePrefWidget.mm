/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Jono Cole

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

#include "FmLastPrefPanePrefWidget.h"
#include <QApplication>
#include <QMacNativeWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPalette>
#include <QColorDialog>
#include <QListWidget>
#include "../Dialogs/SettingsDialog.h"
#include <lastfm/ws.h>
#include "../../../lib/unicorn/UnicornApplication.h"
#include <QDebug>

class MySettingsDialog : public SettingsDialog {
public:
    MySettingsDialog( QWidget* p = 0 )
    :SettingsDialog( p ){ setWindowFlags( Qt::Widget ); ui.pageList->setCurrentRow( 2 ); setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed);}
    
//    virtual QSize sizeHint() const {
//        NSView* nsv = reinterpret_cast<NSView*>(winId());
//        return QSize( [[[nsv superview] superview] frame].size.width, [[[nsv superview] superview] frame].size.height );
//    }
    
protected:
    virtual bool eventFilter(QObject *obj, QEvent *ev) {
        
        if( ev->type() == QEvent::Resize ) {
//            updateGeometry();
            NSView* nsv = reinterpret_cast<NSView*>(winId());
            layout()->update();
            if( [nsv window] )
                [nsv setFrameSize: [[[nsv superview] superview] frame].size ];
        }
        return false;
    }
};

FmLastPrefPanePrefWidget::FmLastPrefPanePrefWidget()
                         :dialog( 0 ), 
                          widget( 0 )
{
    char* argv;
    int argc = 0;
    unicorn::Application* app = new unicorn::Application( argc, &argv );
    QCoreApplication::arguments();
    widget = new QMacNativeWidget();
    QBoxLayout* layout = new QVBoxLayout( widget );

    layout->addWidget( dialog = new MySettingsDialog( widget ) );
    
    foreach( QWidget* w, dialog->findChildren<QWidget*>()) {
        w->installEventFilter( dialog );
        w->show();
    }
    
    NSWindow *systemPrefsWindow = [[NSApplication sharedApplication] mainWindow];
    [reinterpret_cast<NSView*>(dialog->winId()) setFrame: [systemPrefsWindow contentRectForFrameRect:[systemPrefsWindow frame]]];
    [reinterpret_cast<NSView*>(widget->winId()) setAutoresizingMask: NSViewWidthSizable ];

    widget->layout()->setContentsMargins( 0, 0, 0, 0 );
    widget->layout()->setSpacing( 0 );

    dialog->setAutoFillBackground( true );
    
    
    widget->show();
}

NSView* 
FmLastPrefPanePrefWidget::view()
{
    qDebug() << "Getting view..";
    
    const WId id = widget->winId();
    const NSView* ret = reinterpret_cast<NSView*>(id);
    
//    widget->layout()->update();
    return ret;
}

void 
FmLastPrefPanePrefWidget::updateGeometry()
{
    if( this && dialog )
        dialog->updateGeometry();
}
