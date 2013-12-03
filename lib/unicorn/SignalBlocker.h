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

#ifndef SIGNAL_BLOCKER_H_
#define SIGNAL_BLOCKER_H_

#include "lib/DllExportMacro.h"
#include <QTimer>
#include <QEventLoop>

#include "lib/DllExportMacro.h"

/**
  * @brief Waits for a Qt signal to be emitted before
  *        proceeding with code execution.
  * 
  * @author Jono Cole<jono@last.fm>
  */
class UNICORN_DLLEXPORT SignalBlocker : public QEventLoop
{
Q_OBJECT
public:
    SignalBlocker( QObject* o, const char* signal, int timeout)
    : m_ret( true )
    {
        m_timer.setSingleShot( true );
        connect( o, signal, SLOT( onSignaled()));
        
        if( timeout > -1 ) {
            m_timer.setInterval( timeout );
            connect( &m_timer, SIGNAL(timeout()), this, SLOT( onTimeout()));
        }
    }
    
    /** returns true if the signal fired otherwise false if
        the timeout fired */
    bool start()
    {
        if( m_ret == false ) return false;
        m_timer.start();
        m_loop.exec();
        return m_ret;
    }

private slots:
    void onSignaled()
    {
        m_ret = true;
        m_timer.stop();
        m_loop.quit();
    }

    void onTimeout()
    {
        m_ret = false;
        m_loop.quit();
    }

private:
    QEventLoop m_loop;
    bool m_ret;
    QTimer m_timer;
};

#endif //SIGNAL_BLOCKER_H_
