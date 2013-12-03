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
#ifndef PLAYER_CONNECTION_H
#define PLAYER_CONNECTION_H

#include "lib/DllExportMacro.h"
#include "PlayerCommand.h"
#include "State.h"
#include <lastfm/Track.h>
#include <QTimer>
#include <QPointer>


/** delete yourself when the player closes/quits */
class LISTENER_DLLEXPORT PlayerConnection : public QObject
{
    Q_OBJECT
protected:
    QPointer<QTimer> m_stoppedTimer;
    QString const m_id;
    QString const m_name;   
    uint m_elapsed;

    PlayerConnection();
    
    State m_state;
    Track m_track;
    
public:    
    PlayerConnection( const QString& id, const QString& name, QObject* parent = 0 );
    
    ~PlayerConnection()
    {
        bool const wasStopped = m_state == Stopped;
        clear();
        if (!wasStopped)
            emit stopped();
    }

    QString name() const { return m_name; }
    QString id() const { return m_id; }
    virtual Track track() const { return m_track; }
    virtual State state() const { return m_state; }

    /** 0 until we are paused and made non-current by the mediator
     * then we store elapsed_scrobble_time */
    void setElapsed( uint i ) { m_elapsed = i; }
    uint elapsed() const { return m_elapsed; }
    
    void clear() { m_state = Stopped; m_track = Track(); m_elapsed = 0; }
    
    /** only pass the track for CommandStart */
    void handleCommand( PlayerCommand, Track = Track() );    

    void forceTrackStarted( const Track& );
    void forcePaused();
    
signals:
    void trackStarted( const lastfm::Track&, const lastfm::Track& );
    void paused();
    void resumed();
    void stopped();
	void bootstrapReady( const QString& playerId );

private slots:
    void onStopped();
};

#endif
