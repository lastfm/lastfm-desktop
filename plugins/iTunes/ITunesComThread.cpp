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
#ifdef WIN32

#include "ITunesComThread.h"
#include "ITunesPlaysDatabase.h"

#include "ITunesComWrapper.h"
#include "common/c++/Logger.h"
#include "Moose.h"
#include "EncodingUtils.h"

#include <process.h>

#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

// We use a custom message to tell our thread message loop to sync
enum {
	WM_SYNC_PLAYS_DB = WM_USER,
	WM_STOP,
	WM_IS_PLAYING
};
    
/******************************************************************************
    ITunesComThread
******************************************************************************/
ITunesComThread::ITunesComThread() :
    m_com( 0 ),
    m_db( 0 ),
    m_comEnabled( true ),
    m_threadHandle( 0 ),
    m_hWnd( 0 ),
    m_timeOfLastTrackChange( 0 ),
    m_retrying( false ),
    m_retryAttempts( 0 )
{
    InitializeCriticalSection(&m_critSect);

	startComThread();
}


ITunesComThread::~ITunesComThread()
{
    LOGL( 3, "~ITunesComThread()" );

    // Thread shutdown has been initialised already via the OnAboutToQuit event
    DWORD waitResult = WaitForSingleObject( m_threadHandle, 5000 );
    if ( waitResult == WAIT_TIMEOUT || waitResult == WAIT_FAILED )
    {
        LOGL( 2, "Wait for ITunesComThread thread termination " <<
            ( ( waitResult == WAIT_TIMEOUT ) ? "timed out. " : "failed. " ) <<
            "GetLastError(): " << GetLastError() );
    }
    else
    {
        LOGL( 3, "iTunes COM thread terminated" );
    }
	

    CloseHandle( m_threadHandle );
    DeleteCriticalSection( &m_critSect );
}

void
ITunesComThread::startComThread()
{
   unsigned threadId;
    m_threadHandle = reinterpret_cast<HANDLE>(
        _beginthreadex(
            NULL,          // security crap
            0,             // stack size
            threadMain,    // start function
            reinterpret_cast<void*>(this),  // argument to thread
            0,             // run straight away
            &threadId ) ); // thread id out

    if ( m_threadHandle == 0 ) // Error
    {
        // FIXME
        LOGL( 1, "Failed to start iTunes COM thread: " << string( strerror(errno) ) );
    }
}

void
ITunesComThread::syncTrack( const VisualPluginTrack& vpt )
{
	// If the com thread was closed due to a close signal
	// but iTunes didn't actaully close we should start up our thread again
	if ( !m_com )
		startComThread();

    // Needs guarding because it will be read by the worker thread after the
    // PostMessage. If we come into this function twice before the worker thread
    // has had a chance to wake up, it shouldn't matter that we miss we the first
    // track. The reason being that if two or more tracks come in in such quick
    // succession, they have most likely been skipped and their playcounts
    // won't have increased.
    EnterCriticalSection( &m_critSect );
    m_visTrackToSync = vpt;
    LeaveCriticalSection( &m_critSect );
    
    PostMessage( m_hWnd, WM_SYNC_PLAYS_DB, reinterpret_cast<WPARAM>( this ), 0 );
}

void
ITunesComThread::stop()
{
	// If the com thread was closed due to a close signal
	// but iTunes didn't actaully close we should start up our thread again
	if ( !m_com )
		startComThread();
    
    PostMessage( m_hWnd, WM_STOP, reinterpret_cast<WPARAM>( this ), 0 );
}

void
ITunesComThread::callbackIfStopped( void (*callback)() )
{
	if ( !m_com )
		startComThread();

	SendMessageCallback( m_hWnd, WM_IS_PLAYING, reinterpret_cast<WPARAM>( this ), 0, ITunesComThread::isPlayingCallback, reinterpret_cast<WPARAM>( callback ) );
}

bool
ITunesComThread::isPlaying()
{
	bool isPlaying = true;

	try
	{
		m_com->currentTrack();
	}
	catch (...)
	{
		isPlaying = false;
	}

	return isPlaying;
}

VOID
ITunesComThread::isPlayingCallback( HWND hwnd, UINT uMsg, ULONG_PTR dwData, LRESULT isPlaying )
{
	void (*callback)() = reinterpret_cast<void (*)()>( dwData );

	if ( !isPlaying )
		callback();
}


void
ITunesComThread::onDatabaseChanged( std::vector<ITunesEventInterface::ITunesIdSet> deletedObjects,
                                    std::vector<ITunesEventInterface::ITunesIdSet> changedObjects )
{
/*
    for ( int i = 0; i < changedObjects.size(); ++i )
    {
        ITunesEventInterface::ITunesIdSet ids = changedObjects.at( i );
        ITunesTrack t = m_com->track( ids );
    
        if ( !t.isNull() )
        {
            LOGL( 3, "Changed track: " << t.toString() );
        }
    }
*/
}
    

void
ITunesComThread::onPlay( ITunesTrack track )
{
/*
    LOGWL( 3, "onPlay track: " << track.toString() << " " << track.playCount() );
    LOGWL( 3, "onPlay m_lastTrack: " << m_lastTrack.toString() << " " << m_lastTrack.initialPlayCount() );

    if ( !track.isSameAs( m_lastTrack ) )
    {
        if ( m_loopMode )
        {
            // We sync here because otherwise we will miss looped tracks
            // (the cunts don't emit a stopped when they exit the loop).
            //
            // They do however emit a stop if they are skipped. In that case
            // it will have been synced in onStop and the m_lastTrack has been
            // set to null. So only sync here if it's non-null.
            //
            // The remaining flaw in this system is that if the user presses
            // stop on a looped track, we will not get a stop event. Therefore
            // the playcounts for that track will not be synced until the user
            // starts another track.
            if ( !m_lastTrack.isNull() )
            {
                LOGL( 3, "Leaving loop mode, syncing" );

                syncComTrackInThread( m_lastTrack );
            }
        }

        // This stores the current playcount internally at this point
        m_lastTrack = ExtendedITunesTrack::from( track );

        m_loopMode = false;
    }
    else
    {
        // Don't sync when we're looping one track as iTunes event emissions are fucked.
        LOGL( 3, "Same track detected, loopmode = true" );
        m_loopMode = true;
    }
*/
}


void
ITunesComThread::onStop( ITunesTrack )
{
    //LOGWL( 3, "onStop m_lastTrack: " << m_lastTrack.toString() << " " << m_lastTrack.initialPlayCount() );
    //LOGWL( 3, "onStop current track: " << m_com->currentTrack().toString() << " " << m_com->currentTrack().playCount() );
    
    // We compare the current track to last track here so as to prevent syncing when
    // we're in 1x loop mode. Once iTunes changes to a different track we will sync.
    // This conditional will also not hit when the user pressed stop on a track as iTunes
    // considers the stopped track still current (it's still displayed in the iTunes
    // current track window thingie). However, this is not a problem as the playcount
    // hasn't increased in this case, so a sync isn't necessary.

/*
    if ( !m_lastTrack.isSameAs( m_com->currentTrack() ) )
    {
        // We sync here because otherwise we will miss the sync at end of playlist.
        // We blank the m_lastTrack to prevent a double sync from happening when
        // we come into onPlay after leaving loop mode by skipping.
        syncComTrackInThread( m_lastTrack );
        m_lastTrack = ExtendedITunesTrack();    
    }
    else
        LOGL( 3, "Same track, not syncing" );
*/    

    // Why do we clear this here?
    // If we don't, we know that we always have an m_lastTrack in onPlay and can compare
    // to see if we're looping the same track.
    //m_lastTrack = ExtendedITunesTrack();
}

void
ITunesComThread::doStop()
{
    //LOGWL( 3, "onStop m_lastTrack: " << m_lastTrack.toString() << " " << m_lastTrack.initialPlayCount() );
    //LOGWL( 3, "onStop current track: " << m_com->currentTrack().toString() << " " << m_com->currentTrack().playCount() );
    
    // We compare the current track to last track here so as to prevent syncing when
    // we're in 1x loop mode. Once iTunes changes to a different track we will sync.
    // This conditional will also not hit when the user pressed stop on a track as iTunes
    // considers the stopped track still current (it's still displayed in the iTunes
    // current track window thingie). However, this is not a problem as the playcount
    // hasn't increased in this case, so a sync isn't necessary.

	ITunesTrack currentTrack;
	
	try
	{
		currentTrack = m_com->currentTrack();
	}
	catch (...)
	{
	}

    if ( !m_lastTrack.isSameAs( currentTrack ) )
    {
        // We sync here because otherwise we will miss the sync at end of playlist.
        // We blank the m_lastTrack to prevent a double sync from happening when
        // we come into onPlay after leaving loop mode by skipping.
        syncComTrackInThread( m_lastTrack );
        m_lastTrack = ExtendedITunesTrack();    
    }
    else
        LOGL( 3, "Same track, not syncing" );
    

    // Why do we clear this here?
    // If we don't, we know that we always have an m_lastTrack in onPlay and can compare
    // to see if we're looping the same track.
    //m_lastTrack = ExtendedITunesTrack();
}



void
ITunesComThread::onTrackChanged( ITunesTrack track )
{
    // This is fired for "joined CD tracks" (see Advanced menu).
    // However, iTunes doesn't maintain playcounts for CD tracks so we
    // don't need to do any syncing in here.
    
    // The other thing that will cause this event to fire is if the user
    // changed the metadata of a track. This should never cause a playcount
    // change either so no need to sync. We should probably update our
    // database with the new path here though. TODO!

    //LOGWL( 3, "onTrackChanged track: " << track.toString() << " " << track.playCount() );
    //LOGWL( 3, "onTrackChanged m_lastTrack: " << m_lastTrack.toString() << " " << m_lastTrack.initialPlayCount() );
}


void
ITunesComThread::onAboutToPromptUserToQuit()
{
    // Terminate the thread's message loop
    PostQuitMessage( 0 );
}


void
ITunesComThread::onComCallsDisabled()
{
    LOGL( 3, "" );

    m_comEnabled = false;
}


void
ITunesComThread::onComCallsEnabled()
{
    LOGL( 3, "" );

    m_comEnabled = true;

    syncQueue();
}

unsigned
ITunesComThread::eventLoop()
{
    setupWndProc();

    UINT returnCode = 0;
    try
    {
        // This call will cause the bootstrap to happen first time it's called
        m_db = new ITunesPlaysDatabase();
        
        // This might take a long time if iTunes isn't responding to the CoCreateInstance
        m_com = new ITunesComWrapper( this );
        
        try { LOGWL( 3, "iTunes version: " << m_com->iTunesVersion() ); }
        catch ( ITunesException& ) { }

        BOOL messageVal;
        MSG message;
        while ( ( messageVal = GetMessage( &message, NULL, 0, 0 ) ) != 0 )
        {
            if ( messageVal == -1 )
            {
                LOGL( 3, "GetMessage error: " << GetLastError() );
                continue;
            }
            else
            {
                TranslateMessage( &message );
                DispatchMessage( &message );
            }
        }

        // This is the wParam of the WM_QUIT message
        returnCode = static_cast<UINT>( message.wParam );
    }
    catch ( ITunesException& e )
    {
        LOGL( 1, "FATAL ERROR: " << e.what() );
    }

    delete m_com;
	m_com = 0;
    delete m_db;
	m_db = 0;

    LOGL( 3, "Thread shutting down" );

    return returnCode;
}


bool
ITunesComThread::setupWndProc()
{
    WNDCLASSEX wndClass;
    wndClass.cbSize = sizeof( wndClass );
    wndClass.style = NULL;
    wndClass.lpfnWndProc = ITunesComThread::wndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = GetModuleHandle( NULL );
    wndClass.hIcon = NULL;
    wndClass.hCursor = NULL;
    wndClass.hbrBackground = NULL;
    wndClass.lpszMenuName = "";
    wndClass.lpszClassName = "ITunesComThreadClass";
    wndClass.hIconSm = NULL;

    ATOM regClassResult = RegisterClassEx( &wndClass );
    
    if ( regClassResult == 0 )
    {
        LOGL( 1, "RegisterClassEx failed: " << regClassResult );
        LOGL( 1, "Last Error Code: " << GetLastError() );
        return false;
    }
    
    m_hWnd = CreateWindow( "ITunesComThreadClass",
                           "ITunesComThreadWindow",
                           NULL, 0, 0, 0, 0, NULL, NULL, NULL, NULL );
 
    if ( m_hWnd == NULL )
    {
        LOGL( 1, "CreateWindow failed: " << m_hWnd );
        LOGL( 1, "Last Error Code: " << GetLastError() );
        return false;
    }

    return true;
}


// static
LRESULT CALLBACK 
ITunesComThread::wndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if ( uMsg == WM_SYNC_PLAYS_DB )
    {
        ITunesComThread* instance = reinterpret_cast<ITunesComThread*>( wParam );
        instance->syncVisTrackInThread();
        return 0;
    }
	else if ( uMsg == WM_STOP )
    {
        ITunesComThread* instance = reinterpret_cast<ITunesComThread*>( wParam );
        instance->doStop();
        return 0;
    }
	else if ( uMsg == WM_IS_PLAYING )
    {
        ITunesComThread* instance = reinterpret_cast<ITunesComThread*>( wParam );
        return instance->isPlaying();
    }
    else if ( uMsg == WM_TIMER )
    {
        ITunesComThread* instance = reinterpret_cast<ITunesComThread*>( wParam );
        instance->syncVisTrackInThread();
        return 0;
    }
    else
    {
        return DefWindowProc( hwnd, uMsg, wParam, lParam );
    }
}


void
ITunesComThread::syncComTrackInThread( const ExtendedITunesTrack& changedTrack )
{
    try
    {
        if ( !changedTrack.isNull() )
        {
            // This is used if we sync from the COM events (which we currently don't)
            if ( !m_db->sync( changedTrack ) )
            {
                LOGWL( 2, "ITunesComThread failed to update the local database for: "
                         << changedTrack.toString() );
            }

			if ( !m_db->syncOld( changedTrack ) )
            {
                LOGWL( 2, "ITunesComThread failed to update the old local database for: "
                         << changedTrack.toString() );
            }
        }
    }
    catch ( ITunesException& )
    {
        // Ignore this track
    }
}


void
ITunesComThread::syncVisTrackInThread()
{
    if ( m_comEnabled )
    {
        ExtendedITunesTrack currentComTrack;
        try
        {
            // TODO: potential sanity checking of this track against the m_visTrackToSync
            currentComTrack = ExtendedITunesTrack::from( m_com->currentTrack() );

            if ( !m_lastTrack.isNull() )
            {
                // HACK WARNING: all this crap was introduced last minute when we found that iTunes
                // had stopped always returning the new playcount on track change. This logic has
                // got a bit too complicated, refactor.

                DWORD elapsed = GetTickCount() - m_timeOfLastTrackChange;
                
                if ( !m_retrying )
                {
                    // To avoid lots of superfluous sync attempts on repeated skipping, we
                    // don't bother trying to sync if less than 1s has passed.
                    m_timeOfLastTrackChange = GetTickCount();
                    if ( elapsed < ( 1 * 1000 ) )
                    {
                        LOGL( 3, "Less than 1 second since last change, won't sync" );

                        goto refreshCurrentTrack;
                    }
                }
                else
                {
                    // Check if we've been retrying for too long and should give up.
                    if ( elapsed > ( 15 * 1000 ) )
                    {
                        LOGL( 3, "Retried for 15 seconds and still no diff. Give up." );

                        KillTimer( m_hWnd, (UINT_PTR)this );
                        m_retrying = false;

                        goto refreshCurrentTrack;
                    }
                }

                // Check if the diff is 0 and if so, go into retry mode. We have to do this
                // because iTunes doesn't seem to update its internal playcount immediately
                // after the track finished
                int diff = 0;
                try
                {
                    diff = m_lastTrack.playCountDifference();
                }
                catch( PlayCountException& )
                {
                    LOGL( 2, "Playcount Exception when reading initial diff, can't sync" );
                    diff = 0;
                }

                if ( diff == 0 )
                {
                    if ( !m_retrying )
                    {
                        LOGL( 3, "Got a diff of 0, starting retry timer" );
                        SetTimer( m_hWnd, (UINT_PTR)this, 1000, 0 ); // 1-sec timer                
                        m_retrying = true;
                        m_retryAttempts = 0;
                    }
                    else
                    {
                        m_retryAttempts++;
                    }
                    return;            
                }
                else
                {
                    if ( m_retrying )
                        LOGL( 3, "Got a diff of " << diff << " after " << m_retryAttempts << " secs" );

                    KillTimer( m_hWnd, (UINT_PTR)this );
                    m_retrying = false;
                    m_retryAttempts = 0;
                }

                // Sync previous track
                if ( !m_db->sync( m_lastTrack ) )
                {
                    LOGWL( 3, "ITunesComThread failed to update the local database for: "
                             << m_lastTrack.toString() );
                }

				if ( !m_db->syncOld( m_lastTrack ) )
                {
                    LOGWL( 3, "ITunesComThread failed to update the old local database for: "
                             << m_lastTrack.toString() );
                }
            }
        }
        catch ( ITunesException& e )
        {
            LOGL( 2, e.what() );
        }

      refreshCurrentTrack:
        LOGWL( 3, "Current track: " << currentComTrack.toString() );
        m_lastTrack = currentComTrack;

    }
    else // COM is disabled
    {
        if ( m_retrying )
        {
            KillTimer( m_hWnd, (UINT_PTR)this );
            m_retrying = false;
            return;
        }

        // This happens when COM is unavailable due to a modal dialog displaying
        // in iTunes. We store the track on a queue for later processing.
        VisualPluginTrack lastVisTrack;
        EnterCriticalSection( &m_critSect );
        lastVisTrack = m_visTrackToSync;
        LeaveCriticalSection( &m_critSect );

        LOGWL( 3, "COM is disabled, putting " << lastVisTrack.track << " in queue" );
        m_visTrackQueue.push_back( lastVisTrack );
    }
}


void
ITunesComThread::syncQueue()
{
    LOGL( 3, "Size of vis track queue before weeding: " << m_visTrackQueue.size() );

    // Weed out dupes in m_visTrackQueue and only keep the one with the lowest playcount.
    // m_lastTrack also needs to be taken into account.
    VisualPluginTrack current;
    current.path = m_lastTrack.path();
    size_t i = -1; // m_lastTrack is index -1
    while ( true ) 
    {
        // Search for dupes of current track in the remainder (hence i+1) of list
        for ( size_t j = i + 1; j < m_visTrackQueue.size(); /* nowt */ )
        {
            if ( current.path == m_visTrackQueue.at( j ).path )
                m_visTrackQueue.erase( m_visTrackQueue.begin() + j );
            else
                j++;
        }
    
        i++;
        if ( i < m_visTrackQueue.size() )
            current = m_visTrackQueue.at( i );
        else
            break;    
    }

    LOGL( 3, "Size of vis track queue after weeding: " << m_visTrackQueue.size() );


    // Sync tracks that were put on queue while COM was off
    for ( size_t n = 0; n < m_visTrackQueue.size(); ++n )
    {
        try
        {
            VisualPluginTrack vpt = m_visTrackQueue.at( n );
     
            // Now, we need to try and locate this track in the iTunes library
            // to find its current playcount.
            vector<ITunesTrack> results =
                m_com->searchForTrack( ITPlaylistSearchFieldSongNames, vpt.track );

            ITunesTrack ourGuy;
            if ( results.size() == 0 )
            {
                // Eh?
                LOGWL( 2, "Couldn't find matching track in db for " << vpt.track << ". Result size 0." );
                continue;
            }
            else if ( results.size() == 1 )
            {
                // We've found our guy
                ourGuy = results.at( 0 );
            }
            else
            {
                // Need to narrow it down, let's look at the path too
                for ( size_t i = 0; i < results.size(); ++i )
                {
                    ITunesTrack currentResult = results.at( i );
                    if ( currentResult.path() == vpt.path )
                    {
                        // Found him
                        ourGuy = currentResult;
                        break;
                    }
                }
                
                if ( ourGuy.isNull() )
                {
                    // Not much we can do, we'll have to drop the diff for this track
                    LOGWL( 2, "Couldn't find matching track in db for " << vpt.track << ". Path not found." );
                    continue;
                }
            }

            ExtendedITunesTrack diffTrack = ExtendedITunesTrack::from( ourGuy ); // throws

            diffTrack.setInitialPlaycount( vpt.playCount );
            if ( !m_db->sync( diffTrack ) )
            {
                LOGWL( 2, "ITunesComThread failed to update the local database for: "
                         << diffTrack.toString() );
            }

			if ( !m_db->syncOld( diffTrack ) )
            {
                LOGWL( 2, "ITunesComThread failed to update the old local database for: "
                         << diffTrack.toString() );
            }
        }
        catch ( ITunesException& )
        {
            LOGL( 2, "ITunesException when trying to sync one of the tracks in the queue, ignoring track." );
        }

    }
    
    m_visTrackQueue.clear();

    // This will cause the old m_lastTrack which we still have kicking about
    // to get synced and the current track stored in m_lastTrack.
    syncVisTrackInThread();
}

#endif // WIN32

