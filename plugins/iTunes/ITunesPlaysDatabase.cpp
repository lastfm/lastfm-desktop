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
#include "ITunesPlaysDatabase.h"

#include "Moose.h"
#include "common/c++/logger.h"
#include "ITunesTrack.h"

#include <errno.h>
#include <sys/stat.h>

#ifndef WIN32
#include <pthread.h>
#include <unistd.h>   
#endif

#include "sqlite3.h"


// this is a macro to ensure useful location information in the log
#define logError( db ) \
        LOG( 2, "sqlite error: " \
            << sqlite3_errcode( db ) \
            << " (" << sqlite3_errmsg( db ) << ")" );


ITunesPlaysDatabase::ITunesPlaysDatabase()
{
#ifdef WIN32
    std::string path = Moose::wStringToUtf8( Moose::applicationSupport() + L"\\iTunesPlays.db" );
#else
    std::string path = Moose::applicationSupport() + "iTunesPlays.db";
#endif

    if ( sqlite3_open( path.c_str(), &m_db ) != SQLITE_OK )
    {
        LOG( 2, "ERROR: Could not open iTunesPlays Database: " << path );
        logError( m_db );
    }

#ifdef WIN32
    prepare();
#endif
}


void
ITunesPlaysDatabase::prepare()
{
	#ifdef WIN32
		#define BOOTSTRAP_FLAG L"--bootstrap"
	#else
		#define BOOTSTRAP_FLAG "--bootstrap"
	#endif

    // if twiddly is already running, quite probably we're already bootstrapping
    if ( (!isValid() || needsBootstrap()) && !Moose::isTwiddlyRunning() )
		Moose::exec( Moose::twiddlyPath(), BOOTSTRAP_FLAG );
}


ITunesPlaysDatabase::~ITunesPlaysDatabase()
{
    sqlite3_close( m_db ); //docs says passing NULL is harmless noop
}

#ifdef WIN32
/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
typedef struct UnlockNotification UnlockNotification;

struct UnlockNotification
{
	int fired;                         /* True after unlock event has occurred */
	CRITICAL_SECTION mutex;
	CONDITION_VARIABLE cond;
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
static void unlock_notify_cb(void **apArg, int nArg)
{
	for( int i = 0; i < nArg ; i++)
	{
		UnlockNotification *p = (UnlockNotification *)apArg[i];
		EnterCriticalSection( &p->mutex );
		p->fired = 1;
		//WakeConditionVariable( &p->cond );
		LeaveCriticalSection( &p->mutex );
	}
}

/*
** This function assumes that an SQLite API call (either sqlite3_prepare_v2() 
** or sqlite3_step()) has just returned SQLITE_LOCKED. The argument is the
** associated database connection.
**
** This function calls sqlite3_unlock_notify() to register for an 
** unlock-notify callback, then blocks until that callback is delivered 
** and returns SQLITE_OK. The caller should then retry the failed operation.
**
** Or, if sqlite3_unlock_notify() indicates that to block would deadlock 
** the system, then this function returns SQLITE_LOCKED immediately. In 
** this case the caller should not retry the operation and should roll 
** back the current transaction (if any).
*/
static int wait_for_unlock_notify(sqlite3 *db){
	int rc;
	UnlockNotification un;

	/* Initialize the UnlockNotification structure. */
	un.fired = 0;
	InitializeCriticalSection( &un.mutex );
	//InitializeConditionVariable( &un.cond );

	/* Register for an unlock-notify callback. */
	rc = sqlite3_unlock_notify( db, unlock_notify_cb, (void *)&un );

	//assert( rc==SQLITE_LOCKED || rc==SQLITE_OK );

	/* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
	** or SQLITE_OK. 
	**
	** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
	** case this function needs to return SQLITE_LOCKED to the caller so 
	** that the current transaction can be rolled back. Otherwise, block
	** until the unlock-notify callback is invoked, then return SQLITE_OK.
	*/
	if( rc==SQLITE_OK )
	{
		EnterCriticalSection( &un.mutex );

		if( !un.fired )
		{
			//SleepConditionVariableCS( &un.cond, &un.mutex, INFINITE );
		}

		LeaveCriticalSection( &un.mutex );
	}

	/* Destroy the mutex and condition variables. */
	//pthread_cond_destroy(&un.cond);
	DeleteCriticalSection( &un.mutex );

	return rc;
}

#else

/*
 ** A pointer to an instance of this structure is passed as the user-context
 ** pointer when registering for an unlock-notify callback.
 */
typedef struct UnlockNotification UnlockNotification;
struct UnlockNotification {
    int fired;                         /* True after unlock event has occurred */
    pthread_cond_t cond;               /* Condition variable to wait on */
    pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
 ** This function is an unlock-notify callback registered with SQLite.
 */
static void unlock_notify_cb(void **apArg, int nArg){
    int i;
    for(i=0; i<nArg; i++){
        UnlockNotification *p = (UnlockNotification *)apArg[i];
        pthread_mutex_lock(&p->mutex);
        p->fired = 1;
        pthread_cond_signal(&p->cond);
        pthread_mutex_unlock(&p->mutex);
    }
}

/*
 ** This function assumes that an SQLite API call (either sqlite3_prepare_v2()
 ** or sqlite3_step()) has just returned SQLITE_LOCKED. The argument is the
 ** associated database connection.
 **
 ** This function calls sqlite3_unlock_notify() to register for an
 ** unlock-notify callback, then blocks until that callback is delivered
 ** and returns SQLITE_OK. The caller should then retry the failed operation.
 **
 ** Or, if sqlite3_unlock_notify() indicates that to block would deadlock
 ** the system, then this function returns SQLITE_LOCKED immediately. In
 ** this case the caller should not retry the operation and should roll
 ** back the current transaction (if any).
 */
static int wait_for_unlock_notify(sqlite3 *db){
    int rc;
    UnlockNotification un;
    
    /* Initialize the UnlockNotification structure. */
    un.fired = 0;
    pthread_mutex_init(&un.mutex, 0);
    pthread_cond_init(&un.cond, 0);
    
    /* Register for an unlock-notify callback. */
    rc = sqlite3_unlock_notify(db, unlock_notify_cb, (void *)&un);
    assert( rc==SQLITE_LOCKED || rc==SQLITE_OK );
    
    /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED
     ** or SQLITE_OK.
     **
     ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
     ** case this function needs to return SQLITE_LOCKED to the caller so
     ** that the current transaction can be rolled back. Otherwise, block
     ** until the unlock-notify callback is invoked, then return SQLITE_OK.
     */
    if( rc==SQLITE_OK ){
        pthread_mutex_lock(&un.mutex);
        if( !un.fired ){
            pthread_cond_wait(&un.cond, &un.mutex);
        }
        pthread_mutex_unlock(&un.mutex);
    }
    
    /* Destroy the mutex and condition variables. */
    pthread_cond_destroy(&un.cond);
    pthread_mutex_destroy(&un.mutex);
    
    return rc;
}

#endif

bool
ITunesPlaysDatabase::query( /* utf-8 */ const char* statement, std::string* result )
{
    LOG( 3, statement );
    
    int error = 0;
    sqlite3_stmt* stmt = 0;

    try
    {
        const char* tail;
        error = sqlite3_prepare( m_db, statement, static_cast<int>( strlen( statement ) ), &stmt, &tail );
        
        if ( error != SQLITE_OK )
            throw "Could not compile SQL to byte-code";        
        
        int busyCount = 0;
        while( error != SQLITE_DONE && busyCount < 20 )
        {
            error = sqlite3_step( stmt );

            switch( error )
            {
                case SQLITE_BUSY:
                    busyCount++;
                    sqlite3_sleep( 25 );
                    break;

                case SQLITE_MISUSE:
                    throw "Could not execute SQL - SQLITE_MISUSE";

                case SQLITE_ERROR:
                    throw "Could not execute SQL - SQLITE_ERROR";

                case SQLITE_ROW:
                    if ( sqlite3_column_count( stmt ) > 0 && result )
                    {
                        *result = (const char*) sqlite3_column_text( stmt, 0 );
                        LOG( 3, "SQLite result: `" << *result << '\'' );
                    }
                    break;

                case SQLITE_DONE:
                    break;

				case SQLITE_LOCKED:
					LOG( 3, "Database locked. Waiting for unlock." );
					busyCount++;
                    sqlite3_sleep( 25 );
					//wait_for_unlock_notify( m_db );
					break;

                default:
                    throw "Unhandled sqlite3_step() return value";
            }
        }
        
        if ( busyCount )
        {
            if ( busyCount >= 20 )
                throw "Could not execute SQL - SQLite was too busy";
            
            LOG( 3, "SQLite was busy " << busyCount << " times" );
        }
    }
    catch( const char* message )
    {
        LOG( 2, "ERROR: " << message );
        logError( m_db );
    }

    sqlite3_finalize( stmt );

    return error == SQLITE_DONE;
}


#include "common/c++/fileCreationTime.cpp"


bool
ITunesPlaysDatabase::isValid()
{
    if (!query( "SELECT COUNT( * ) FROM " TABLE_NAME " LIMIT 0, 1;" ))
        return false;

    // uninstallation protection
    // we store the ctime in the database, so if the database is removed we
    // don't have this value still stored, which may cause additional issues
    // the principle is, if the plugin is uninstalled, then reinstalled sometime
    // the ctime will be different to what we stored at the time of bootstrap
    // the reason for all this is, if there was a period of uninstallation
    // in which the db wasn't removed (happens everytime on Windows), we don't
    // want to scrobble everything played since the uninstallation!
    std::string r;
    query( "SELECT value FROM metadata WHERE key='plugin_ctime';", &r );

    long const stored_time = std::strtol( r.c_str(), 0, 10 /*base*/ );
    time_t actual_time = common::fileCreationTime( Moose::pluginPath() );

    LOG( 3, "Plugin binary ctime: " << actual_time );

    if (errno == EINVAL || stored_time != actual_time)
    {
        LOG( 3, "Uninstallation protection triggered; forcing bootstrap" );
        return false;
    }

    return true;
}


bool
ITunesPlaysDatabase::needsBootstrap()
{
    std::string out;
    bool b = query( "SELECT value FROM metadata WHERE key='bootstrap_complete'", &out );
    if (!b || out != "true")
        return true;

   
#ifndef WIN32
    b = query( "SELECT value FROM metadata WHERE key='schema'", &out );
    if (!b || out != "2")
        return true;
#endif

    return false;
}


int
ITunesPlaysDatabase::playCount( const ITunesTrack& track )
{
#ifdef WIN32
	std::string id = Moose::wStringToUtf8( track.persistentId() );
#else
	std::string id = track.persistentId();
#endif
    char* format = "SELECT play_count FROM " TABLE_NAME " WHERE persistent_id='%q'";

    char* token = sqlite3_mprintf( format, id.c_str() );
    std::string result;
    
    if ( !query( token, &result ) )
        return -1;
    
    long c = std::strtol( result.c_str(), 0, 10 /*base*/ );
    if ( errno == EINVAL )
        return -1;
    return c;
}

int
ITunesPlaysDatabase::playCountOld( const ITunesTrack& track )
{
#ifdef WIN32
    std::string id = Moose::wStringToUtf8( track.path() );
	char* format = "SELECT play_count FROM itunes_db WHERE path='%q'";
#else
	std::string id = track.persistentId();
	char* format = "SELECT play_count FROM itunes_db WHERE persistent_id='%q'";
#endif

    char* token = sqlite3_mprintf( format, id.c_str() );
    std::string result;
    
    if ( !query( token, &result ) )
        return -1;
    
    long c = std::strtol( result.c_str(), 0, 10 /*base*/ );
    if ( errno == EINVAL )
        return -1;
    return c;
}
