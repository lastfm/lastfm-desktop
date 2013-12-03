/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole, Erik Jaelevik, 
        Christian Muehlhaeuser

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
#ifndef ITUNES_PLAYS_DATABASE_H
#define ITUNES_PLAYS_DATABASE_H

#ifdef WIN32
	#include "ITunesTrack.h"
#endif

#include <string>

extern "C"
{
    typedef struct sqlite3 sqlite3;
}

#define TABLE_NAME "playcounts"

/** @author Christian Muehlhaeuser <chris@last.fm>
  * @contributor Jono Cole <jono@last.fm>
  * @contributor Max Howell <max@last.fm>
  * @contributor <erik@last.fm>
  */
class ITunesPlaysDatabase
{
public:
    ITunesPlaysDatabase();
    ~ITunesPlaysDatabase();
    
    /** @returns false if the tables aren't valid or created */
    bool isValid();
    /** @returns true if the db has no valid bootstrap */
    bool needsBootstrap();
    
    /** tracks unknown to us return -1, which means you should verify the 
      * playCount with iTunes before doing anything with that track generally */
    int playCount( const class ITunesTrack& track );

	/** tracks unknown to us return -1, which means you should verify the 
      * playCount with iTunes before doing anything with that track generally */
    int playCountOld( const class ITunesTrack& track );

    /** call to sync the just finished playing track, NOTE the implementation
      * stores what is playing now for the next call to sync() so always call
      * this every new track
      */
    static void sync();

#ifndef WIN32    
    /** general init, and if necessary, creates tables, bootstraps db, etc. */
    static void init();
    
    /** cleans up, call when plugin is unloaded */
    static void finit();
#else
	bool sync( const ExtendedITunesTrack& );
	bool syncOld( const ExtendedITunesTrack& );
#endif

protected:
    /** prepares us for iPod scrobbling, ie creates the tables and bootstraps
      * the database */
    void prepare();
    
    bool query( /* utf-8 */ const char* statement, std::string* result = 0 );
    
    sqlite3* m_db;
    
#ifndef WIN32
    static void* onPlayStateChanged( void* );
    static void* sync( void* );
    static void* syncOld( void* );
    
    static pthread_mutex_t s_mutex;
#endif
};


#endif //ITUNES_PLAYS_DATABASE_H
