/*
   Copyright 2012 Last.fm Ltd.
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

#ifndef UNICORNAPPLICATIONDELEGATE_H
#define UNICORNAPPLICATIONDELEGATE_H

#include <QPixmap>

#include "lib/DllExportMacro.h"

namespace unicorn {

class UnicornApplicationDelegateCommandObserver
{
public:
    virtual QString trackTitle() const = 0;
    virtual QString artist() const = 0;
    virtual QString album() const = 0;
    virtual int duration() = 0;
    virtual QPixmap artwork() = 0;
    virtual bool loved() = 0;
};

class UNICORN_DLLEXPORT UnicornApplicationDelegate : public QObject
{
    Q_OBJECT
public:
    explicit UnicornApplicationDelegate( QObject *parent = 0 );

    void showDockIcon( bool show );

    void setCommandObserver( UnicornApplicationDelegateCommandObserver* observer );

    void forceInitialize();
    void forceApplicationDidFinishLaunching( void* aNotification );
    
signals:
    void initialize();
    void applicationDidFinishLaunching( void* aNotification ); /* NSNotification */
    
public:
    UnicornApplicationDelegateCommandObserver* m_observer;
};

}

#endif // UNICORNAPPLICATIONDELEGATE_H
