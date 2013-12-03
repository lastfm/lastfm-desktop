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

#include "ITunesComWrapper.h"
#include "plugins/scrobsub/EncodingUtils.h"
#include "common/c++/Logger.h"
#include <comutil.h>
#include <process.h>
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

TheAtlModule* g_atlModule = NULL;

   
/******************************************************************************
    ITunesComWrapper
******************************************************************************/
ITunesComWrapper::ITunesComWrapper(
    ITunesEventInterface* handler ) :
        m_iTunesApp( 0 ),
        m_allTracks( 0 ),
        m_trackCount( -1 ),
        m_sink( 0 ),
        m_sinkAsUnknown( 0 ),
        m_sinkId( 0 )
{
    initialiseCom(); // throws

    if ( handler )
    {
        try
        { 
            connectSink( handler );
        }
        catch ( ITunesException& )
        {
            // Just carry on if this fails, we'll get no events but the other stuff will work
        }
    }

    libraryTrackCount(); // throws
}


ITunesComWrapper::~ITunesComWrapper()
{
    uninitialiseCom();
}
    

long
ITunesComWrapper::libraryTrackCount()
{
    // Can't use isValid here as it also checks the m_trackCount and m_allTracks
    // which this method initialises when called from the ctor. Bad class design.
    if ( m_iTunesApp == 0 )
        throw ITunesException( "COM not initialised" );

    IITLibraryPlaylist* playlist;
    HRESULT res = m_iTunesApp->get_LibraryPlaylist( &playlist );
    handleComResult( res, L"Failed to get iTunes library playlist" );

    if ( !m_allTracks )
    {
        res = playlist->get_Tracks( &m_allTracks );
        try
        {
            handleComResult( res, L"Failed to get iTunes library track collection" );
        }
        catch ( ITunesException& )
        {
            m_allTracks = 0;
            playlist->Release();
            throw;
        }
    }

    res = m_allTracks->get_Count( &m_trackCount );
    try
    {
        handleComResult( res, L"Failed to get iTunes library track count" );
    }
    catch ( ITunesException& )
    {
        m_trackCount = -1;
        playlist->Release();
        throw;
    }

    playlist->Release();

    return m_trackCount;
}

void
ITunesComWrapper::setPersistentIDForTrack( ITunesTrack& t, IITTrack* track )
{
    VARIANT variant;
    VariantInit( &variant );
    variant.pdispVal = reinterpret_cast<IDispatch*>( track );
    variant.vt = VT_DISPATCH;

    long high = 0;
    long low = 0;
    HRESULT res = m_iTunesApp->get_ITObjectPersistentIDHigh( &variant, &high );
    if ( res == S_OK )
        res = m_iTunesApp->get_ITObjectPersistentIDLow( &variant, &low );

    if ( res == S_OK )
    {
        wstringstream ss;
        ss.fill( '0' );
        ss.width( 8 );
        ss << hex << high << low;
        t.setPersistentId( ss.str() );
    }
    else
    {
        LOGW( 2, "Failed to get persistent ID" );
    }
}

ITunesTrack
ITunesComWrapper::track( long idx )
{
    if ( !isValid() )
        throw ITunesException( "COM not initialised" );

    IITTrack* track;
    HRESULT res = m_allTracks->get_Item( idx + 1, &track ); // COM arrays are 1-based
    handleComResult( res, L"Failed to get track" );
        
    // IITTrack pointer is Released by the ITunesTrack.
    // Won't throw if the track has no path.
    ITunesTrack t( track );
    setPersistentIDForTrack( t, track );
    return t;
}


IITPlaylist*
ITunesComWrapper::iPodLibraryPlaylist()
{
    HRESULT h;
    #define HANDLE_COM_FAIL( x ) h = ( x ); handleComResult( h, L#x )

    IITSourceCollection* sources;
    HANDLE_COM_FAIL( m_iTunesApp->get_Sources( &sources ) );

    long n;
    try
    {
        HANDLE_COM_FAIL( sources->get_Count( &n ) );
    }
    catch ( ITunesException& )
    {
        sources->Release();
        throw;
    }

    IITPlaylist* playlist = NULL;
    for( int i = 1; i < n + 1 && !playlist; ++i )
    {
        IITSource* source = NULL;
        IITPlaylistCollection* playlists = NULL;
        try
        {
            HANDLE_COM_FAIL( sources->get_Item( i, &source ) );

            ITSourceKind kind;
            HANDLE_COM_FAIL( source->get_Kind( &kind ) );
            
            if ( kind == ITSourceKindIPod )
            {
                HANDLE_COM_FAIL( source->get_Playlists( &playlists ) );
                            
                HANDLE_COM_FAIL( playlists->get_Item( 1, &playlist ) );

                BSTR name;
                HANDLE_COM_FAIL( playlist->get_Name( &name ) );

                LOGW( 3, "Playlist name: " << bstrToWString( name ) );
            }
        }
        catch ( ITunesException& )
        {
            // We'll just continue to the next source
        }
        
        if ( source != NULL ) source->Release();
        if ( playlists != NULL ) playlists->Release();
    }

    sources->Release();

    if ( playlist == NULL )
        throw ITunesException( "No iPod library found" );

    return playlist;
}


long
ITunesComWrapper::iPodLibraryTrackCount()
{
    IITPlaylist* playlist = iPodLibraryPlaylist();

    HRESULT res = playlist->get_Tracks( &m_allTracks );
    try
    {
        handleComResult( res, L"Failed to get iPod library track collection" );
    }
    catch ( ITunesException& )
    {
        m_allTracks = 0;
        playlist->Release();
        throw;
    }

    m_trackCount = -1;
    res = m_allTracks->get_Count( &m_trackCount );
    try
    {
        handleComResult( res, L"Failed to get iPod library track collection" );
    }
    catch ( ITunesException& )
    {
        playlist->Release();
        throw;
    }

    playlist->Release();

    return m_trackCount;
}


ITunesTrack
ITunesComWrapper::track( const ITunesEventInterface::ITunesIdSet& ids )
{
    if ( !isValid() )
        throw ITunesException( "COM not initialised" );

    IITObject* obj;
    HRESULT res = m_iTunesApp->GetITObjectByID( ids.sourceId, ids.playlistId, ids.trackId, ids.dbId, &obj );
    handleComResult( res, L"Failed to get object from ID set" );
        
    IITTrack* track = 0;
    res = obj->QueryInterface( IID_IITTrack, (void**)&track );
    if ( res != S_OK || track == 0 )
    {
        // Not a track object, ignore
        obj->Release();
        handleComResult( res, L"ID set didn't represent a track" );
    }

    // IITTrack pointer is Released by the ITunesTrack.
    // This ctor can't throw.
    ITunesTrack t( track );
    setPersistentIDForTrack( t, track );
    return t;
}

    
ITunesTrack
ITunesComWrapper::currentTrack()
{
    if ( !isValid() )
        throw ITunesException( "COM not initialised" );

    IITTrack* track = NULL;
    HRESULT res = m_iTunesApp->get_CurrentTrack( &track );
    handleComResult( res, L"Failed to get current track" );
        
    // IITTrack pointer is Released by the ITunesTrack
    // This ctor can't throw.
    ITunesTrack t( track );
    setPersistentIDForTrack( t, track );
    return t;
}


ITPlayerState
ITunesComWrapper::playerState()
{
    if ( !isValid() )
        throw ITunesException( "COM not initialised" );

    ITPlayerState state;
    HRESULT res = m_iTunesApp->get_PlayerState( &state );
    handleComResult( res, L"Failed to get player state" );

    return state;
}


long
ITunesComWrapper::playerPosition()
{
    if ( !isValid() )
        throw ITunesException( "COM not initialised" );

    long pos;
    HRESULT res = m_iTunesApp->get_PlayerPosition( &pos );
    handleComResult( res, L"Failed to get player pos" );
        
    return pos;
}


wstring
ITunesComWrapper::iTunesVersion()
{
    if ( !isValid() )
        return L"";

    BSTR v;
    HRESULT res = m_iTunesApp->get_Version( &v );
    if ( !handleComResult( res, L"Failed to get iTunes version" ) )
        return L"";
        
    return bstrToWString( v );
}


vector<ITunesTrack>
ITunesComWrapper::searchForTrack( ITPlaylistSearchField searchField, wstring searchTerm )
{
    vector<ITunesTrack> results;

    if ( !isValid() )
        throw ITunesException( "COM not initialised" );

    IITLibraryPlaylist* playlist;
    HRESULT res = m_iTunesApp->get_LibraryPlaylist( &playlist );
    handleComResult( res, L"Failed to get iTunes library playlist" );

    _bstr_t bTerm( searchTerm.c_str() );
    IITTrackCollection* tc;
    res = playlist->Search( bTerm, searchField, &tc );
    try
    {
        handleComResult( res, L"Failed to search iTunes library" );
    }
    catch ( ITunesException& )
    {
        playlist->Release();
        throw;
    }
        
    if ( tc == 0 )
    {
        playlist->Release();
        return results;
    }
        
    long count;
    tc->get_Count( &count );
    try
    {
        handleComResult( res, L"Failed to get count of search results" );
    }
    catch ( ITunesException& )
    {
        playlist->Release();
        throw;
    }
    
    for ( long i = 1; i < ( count + 1 ); ++i )
    {
        try
        {
            IITTrack* track; 
            tc->get_Item( i, &track );
            handleComResult( res, L"Failed to get track out of search results" );

            ITunesTrack t( track );
			setPersistentIDForTrack( t, track );
            results.push_back( t );
        }
        catch ( ITunesException& )
        {
            // Just move on to the next
        }
    }
    
    playlist->Release();
    return results;
}


void
ITunesComWrapper::initialiseCom()
{
    LOG( 3, "Calling CoInitializeEx..." );

    // Init COM
    HRESULT res = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );

    // S_FALSE means COM has already been initialised on this thread.
    if ( res != S_OK && res != S_FALSE )
    {
        LOG( 2, "Failed to initialise COM. Return: " << res );
    }
    else
    {
        bool retry = false;
        do
        {
            LOG( 3, "Calling CoCreateInstance..." );

            HRESULT res = 0;
            res = CoCreateInstance( CLSID_iTunesApp, // CLSID
                                    NULL, // not part of aggregate
                                    CLSCTX_LOCAL_SERVER, // server is on same machine
                                    IID_IiTunes, // specifies what interface we want the pointer to adhere to (a COM object can implement more than one)
                                    (PVOID*)&m_iTunesApp ); // the pointer to fill

            if ( res == S_OK )
            {
                // We're all set
                retry = false;
            }
            else if ( res == CO_E_SERVER_EXEC_FAILURE )
            {
                // This seems to happen if there is a modal dialog being shown in iTunes
                // or some other delay has occurred. Retrying should do the trick.
                LOG( 2, "Got CO_E_SERVER_EXEC_FAILURE, retrying..." );
                retry = true;
            }
            else
            {
                logComError( res, L"Failed to get hold of iTunes instance via COM" );
                m_iTunesApp = 0;
                retry = false;
            }
        
        } while ( retry );
        
        if ( m_iTunesApp == 0 )
            throw ITunesException( "Failed to get hold of iTunes instance via COM" );

        LOG( 3, "Value of iTunes ptr: " << m_iTunesApp );
    }
}


void
ITunesComWrapper::connectSink( ITunesEventInterface* handler )
{
    LOG( 3, "Connecting sink" );
    
    // Init COM the ATL way, needed for the CComObject to work
    g_atlModule = new TheAtlModule();

    // Instantiate our sink object. By wrapping it in a CComObject, we get all
    // the IUnknown plumbing for free.
    HRESULT res = CComObject<ITunesComEventSink>::CreateInstance( &m_sink );
    handleComResult( res, L"Failed to create event sink object" );

    m_sink->setHandler( handler );

    // Query our own sink object to see if it supports the IUnknown interface (which it
    // obviously does) and return a reference to it as an IUnknown*.
    res = m_sink->QueryInterface( IID_IUnknown, reinterpret_cast<void**>( &m_sinkAsUnknown ) );
    handleComResult( res, L"Getting IUnknown from ITunesComEventSink failed" );

    // Hook up the outgoing events interface from iTunes with our own sink, and get hold
    // of the ID so we can unadvise at shutdown.
    res = AtlAdvise( m_iTunesApp, m_sinkAsUnknown, DIID__IiTunesEvents, &m_sinkId );
    handleComResult( res, L"AtlAdvise failed" );
}


void
ITunesComWrapper::uninitialiseCom()
{
    LOG( 3, "Uninitialising COM..." );

    // Don't do anything unless we managed to initialise the iTunes pointer correctly
    if ( m_iTunesApp != 0 )
    {
        // Release everything in reverse order to what we initialised it in
        if ( m_sinkId != 0 )
        {
            HRESULT res = AtlUnadvise( m_iTunesApp, DIID__IiTunesEvents, m_sinkId );
            try
            {
                handleComResult( res, L"AtlUnadvise failed but what can we do except sigh, shrug our shoulders and move on." );
            }
            catch ( ITunesException& )
            {}
        }

        if ( m_sinkAsUnknown != 0 )
            m_sinkAsUnknown->Release();

        if ( g_atlModule != 0 )
        {
            delete g_atlModule;
            g_atlModule = 0;
        }

        if ( m_allTracks != 0 )
            m_allTracks->Release();

        m_iTunesApp->Release();
    }

    // Close COM, returns no error
    CoUninitialize();

    LOG( 3, "CoUninitialize done" );
}


wstring
ITunesComWrapper::bstrToWString( BSTR bstr )
{
    if ( bstr == 0 )
        return wstring();
    else
    {
        // This wraps the BSTR and the _b_str_t class will take care of
        // deallocating its memory so we don't have to do it.
        _bstr_t bstrCls( bstr, false );
        return wstring( bstrCls );
    }

/* We used to convert it to Utf8
    unsigned int len = bstrCls.length();

    char* utf8buf = new char[len * 4];
    memset( utf8buf, 0, len * 4 );

    // _bstr_t has an operator wchar_t*() which will be used here
    EncodingUtils::UnicodeToUtf8( bstrCls, len, utf8buf, len * 4 );
    string s( utf8buf );

    delete[] utf8buf;
    
    return s;
*/
}

// HACK: This shouldn't be here but there isn't really anywhere else to put
// it right now unless we want to create a utility class inside common.
string
wstring2string( wstring ws )
{
    string s;
    for ( size_t i = 0; i < ws.size(); ++i )
    {
        unsigned short us = ws.at( i );
        char c = (char)us;
        s += c;
    }
    return s;
}


bool
ITunesComWrapper::handleComResult( HRESULT res, wstring err )
{
    if ( res == S_OK )
    {
        return false;
    }
    else
    {
        if ( err.empty() )
            err = L"Generic COM error";
        logComError( res, err );
        throw ITunesException( wstring2string( err ).c_str() );
        return true;
    }
}


void
ITunesComWrapper::logComError( HRESULT res, wstring err )
{
    wstring sRes;
    switch ( res )
    {
        case S_OK:                     sRes = L"S_OK"; break;
        case S_FALSE:                  sRes = L"S_FALSE"; break;
        case E_POINTER:                sRes = L"E_POINTER"; break;
        case ITUNES_E_OBJECTDELETED:   sRes = L"ITUNES_E_OBJECTDELETED"; break;
        case ITUNES_E_OBJECTLOCKED:    sRes = L"ITUNES_E_OBJECTLOCKED"; break;
        case E_FAIL:                   sRes = L"E_FAIL"; break;
        case REGDB_E_CLASSNOTREG:      sRes = L"REGDB_E_CLASSNOTREG"; break;
        case CLASS_E_NOAGGREGATION:    sRes = L"CLASS_E_NOAGGREGATION"; break;
        case E_NOINTERFACE:            sRes = L"E_NOINTERFACE"; break;

        // Happens when CoCreateInstance times out for whatever reason
        case CO_E_SERVER_EXEC_FAILURE: sRes = L"CO_E_SERVER_EXEC_FAILURE"; break;

        // We've lost the connection to iTunes
        case CO_E_OBJNOTCONNECTED:     sRes = L"CO_E_OBJNOTCONNECTED"; break;

        // Happens when iTunes displays dialog
        case RPC_E_CALL_REJECTED:      sRes = L"RPC_E_CALL_REJECTED"; break;

        // Happens when iTunes is shut down while we're accessing it
        // (couldn't find the enum value in winerror.h)
        case (HRESULT)0x800706BA:      sRes = L"RPC_S_SERVER_UNAVAILABLE"; break;

        default:
            wchar_t buf[50];
            swprintf_s( buf, 50, L"%x", static_cast<unsigned long>( res ) );
            sRes = wstring( buf );
    }
    
    wostringstream os;
    os << L"COM error: " << err << L"\n  Result code: " << sRes;
    LOGW( 2, os.str() );
}


/******************************************************************************
    ITunesComEventSink
******************************************************************************/
HRESULT
ITunesComEventSink::Invoke( DISPID dispidMember,       // the event in question
                            REFIID /*riid*/,           // reserved - NULL
                            LCID /*lcid*/,             // locale
                            WORD /*wFlags*/,           // context of call
                            DISPPARAMS* pdispparams,   // arguments
                            VARIANT* /*pvarResult*/,   // return value - NULL
                            EXCEPINFO* /*pexcepinfo*/, // out param for exception info
                            UINT* /*puArgErr*/ )       // out param for which param was wrong
{
    // TODO: figure out if I need to free the DISPPARAMS objects

    #define EVENT_ERROR( name )                                                              \
    {                                                                                        \
        if ( pdispparams == 0 )                                                              \
        {                                                                                    \
            LOG( 2, #name " event pdispparams NULL" );                                      \
            hr = DISP_E_PARAMNOTFOUND;                                                       \
        }                                                                                    \
        else                                                                                 \
        {                                                                                    \
            LOG( 2, #name " event returned wrong number of args: " << pdispparams->cArgs ); \
            hr = DISP_E_BADPARAMCOUNT;                                                       \
        }                                                                                    \
    }

    HRESULT	hr = S_OK;

    ITEvent evt = (ITEvent)( dispidMember );
    switch ( evt )
    {
        case ITEventDatabaseChanged:
        {
            //LOG( 3, "ondbchanged" );

            // OnDatabaseChanged should have two parameters
            if ( pdispparams != 0 && pdispparams->cArgs == 2 )
            {
                // IDispatch arguments are passed in reverse order
                onDatabaseChangedEvent( pdispparams->rgvarg[1], pdispparams->rgvarg[0] );
            }
            else
                EVENT_ERROR( OnDatabaseChanged );
        }
        break;

        case ITEventPlayerPlay:
        {
            //LOG( 3, "onplay" );

            // Should have one parameter
            if ( pdispparams != 0 && pdispparams->cArgs == 1 )
            {
                IDispatch* d = pdispparams->rgvarg[0].pdispVal;
                if ( d == 0 )
                {
                    LOG( 2, "d null in onPlay, sending empty track" );

                    // iTunes can return 0 here, so we use a null track
                    m_handler->onPlay( ITunesTrack() );
                }
                else
                {
                    IITTrack* track = 0;
                    HRESULT res = d->QueryInterface( IID_IITTrack, (void**)&track );
                    if ( res == S_OK )
                    {
                        m_handler->onPlay( ITunesTrack( track ) );
                    }
                }
            }
            else
                EVENT_ERROR( OnPlayerPlay );
        }
        break;

        case ITEventPlayerStop:
        {
            //LOG( 3, "onstop" );

            // Should have one parameter
            if ( pdispparams != 0 && pdispparams->cArgs == 1 )
            {
                IDispatch* d = pdispparams->rgvarg[0].pdispVal;
                if ( d == 0 )
                {
                    LOG( 2, "d null in onStop, sending empty track" );
                    
                    // iTunes can return 0 here, so we use a null track
                    m_handler->onStop( ITunesTrack() );
                }
                else
                {
                    IITTrack* track = 0;
                    HRESULT res = d->QueryInterface( IID_IITTrack, (void**)&track );
                    if ( res == S_OK )
                    {
                        m_handler->onStop( ITunesTrack( track ) );
                    }
                }
            }
            else
                EVENT_ERROR( OnPlayerStop );
        }
        break;

        case ITEventPlayerPlayingTrackChanged:
        {
            //LOG( 3, "ontrackchanged" );

            // Should have one parameter
            if ( pdispparams != 0 && pdispparams->cArgs == 1 )
            {
                IDispatch* d = pdispparams->rgvarg[0].pdispVal;
                if ( d == 0 )
                {
                    LOG( 2, "d null in onTrackChanged, sending empty track" );

                    // iTunes can return 0 here, so we use a null track
                    m_handler->onTrackChanged( ITunesTrack() );
                }
                else
                {
                    IITTrack* track = 0;
                    HRESULT res = d->QueryInterface( IID_IITTrack, (void**)&track );
                    if ( res == S_OK )
                    {
                        m_handler->onTrackChanged( ITunesTrack( track ) );
                    }
                }
            }
            else
                EVENT_ERROR( OnPlayingTrackChanged );
        }
        break;

        case ITEventCOMCallsDisabled:
            m_handler->onComCallsDisabled();
        break;

        case ITEventCOMCallsEnabled:
            m_handler->onComCallsEnabled();
        break;

        case ITEventQuitting:
        break;

        case ITEventAboutToPromptUserToQuit:
            m_handler->onAboutToPromptUserToQuit();
        break;

        case ITEventSoundVolumeChanged:
        break;

        default:
            hr = DISP_E_MEMBERNOTFOUND;
        break;
    }
        
    return hr;
}


void
ITunesComEventSink::onDatabaseChangedEvent( VARIANT deletedObjectIDs,
                                            VARIANT changedObjectIDs )
{
    // TODO: can this really not go wrong? No docs about what happens when it fails.
    // Tried forcing an exception in a release build but nothing happened.
    CComSafeArray<VARIANT> deleted( deletedObjectIDs.parray );
    CComSafeArray<VARIANT> changed( changedObjectIDs.parray );
    std::vector<ITunesEventInterface::ITunesIdSet> vecDeleted;
    std::vector<ITunesEventInterface::ITunesIdSet> vecChanged;
    
    LOG( 3, "Deleted: " << deleted.GetCount() << ", Changed: " << changed.GetCount() );

    for ( ULONG i = 0; i < deleted.GetCount(); ++i )
    {
        ITunesEventInterface::ITunesIdSet ids = getIdsFromSafeArray( i, deleted );
        vecDeleted.push_back( ids );
    }
    
    for ( ULONG i = 0; i < changed.GetCount(); ++i )
    {
        ITunesEventInterface::ITunesIdSet ids = getIdsFromSafeArray( i, changed );
        vecChanged.push_back( ids );
    }
    
    m_handler->onDatabaseChanged( vecDeleted, vecChanged );
}    


ITunesEventInterface::ITunesIdSet
ITunesComEventSink::getIdsFromSafeArray( long index, 
                                         CComSafeArray<VARIANT>& array )
{
    VARIANT elem;
    LONG idx2d[2];

    idx2d[0] = index;
    idx2d[1] = 0;
    array.MultiDimGetAt( idx2d, elem );
    long sourceId = elem.lVal;

    idx2d[1] = 1;
    array.MultiDimGetAt( idx2d, elem );
    long playlistId = elem.lVal;

    idx2d[1] = 2;
    array.MultiDimGetAt( idx2d, elem );
    long trackId = elem.lVal;

    idx2d[1] = 3;
    array.MultiDimGetAt( idx2d, elem );
    long dbId = elem.lVal;

    ITunesEventInterface::ITunesIdSet ids = { sourceId, playlistId, trackId, dbId };
    return ids;
}


// Old crap, but leaving the code here for now for reference purposes
// ------------------------------------------------------------------
//void
//ITunesComWrapper::buildMapping()
//{
//    //stdext::hash_map<long, string> mapping; 
//
//    cout << "Starting building map" << endl;
//
//    DWORD startTime = GetTickCount();
//
//    // iterate over itunes collection
//    IITLibraryPlaylist* playlist;
//    m_iTunesApp->get_LibraryPlaylist( &playlist );
//
//    IITTrackCollection* coll;
//    playlist->get_Tracks( &coll );
//        
//    long cnt;
//    coll->get_Count( &cnt );
//
//    for ( long i = 1; i <= cnt; ++i ) // arrays are 1-based in COM
//    {
//        IITTrack* track;
//        coll->get_Item( i, &track );
//        
//        long dbId;
//        track->get_TrackDatabaseID( &dbId );
//        
//        string path = pathForTrack( track );
//        
//        //ostringstream os;
//        //os << "ID: " << dbId << "\nPath: " << path;
//        //Log( os.str() );
//        
//        //mapping.insert( make_pair( dbId, path ) ); 
//    }
//
//    DWORD finishTime = GetTickCount();
//    DWORD elapsed = finishTime - startTime;
//
//    ostringstream os;
//    os << "Finished building mapping, time: " << elapsed;
//    Log( os.str() );
//
//    std::cout << os.str().c_str();
//    
//
//    // get runtime id and path for each track
//    // put these in hash table
//}
//
//
//string
//ITunesComWrapper::findPersistentIdForTrack( IITTrack* track )
//{
//    // Get the metadata from the track
//    BSTR bstrTitle = 0;
//    BSTR bstrArtist = 0;
//    BSTR bstrAlbum = 0;
//    bool gotMetadata =
//        handleComResult( track->get_Name( &bstrTitle ) ) &&
//        handleComResult( track->get_Artist( &bstrArtist ) ) &&
//        handleComResult( track->get_Album( &bstrAlbum ) );
//    if ( !gotMetadata )
//    {
//        Log( "Failed to get metadata" );
//        return "";
//    }
//    
//    string titleUtf8 = bstrToUtf8( bstrTitle );
//    string artistUtf8 = bstrToUtf8( bstrArtist );
//    string albumUtf8 = bstrToUtf8( bstrAlbum );
//
//    // Get the path to the XML
//    BSTR bstrPath;
//    HRESULT res = m_iTunesApp->get_LibraryXMLPath( &bstrPath );
//    if ( res != S_OK )
//    {
//        logComError( "COM failed to get library XML path", res );
//        return "";
//    }
//
//    // Try opening it...
//    _bstr_t path( bstrPath );
//
///*
//    FILE* fp;
//    errno_t err = _wfopen_s( &fp, path, "r" );
//    if ( err != 0 )
//    {
//        char buf[100];
//        strerror_s( buf, 100, err );
//        ostringstream os;
//        os << "Failed to open iTunes XML file. Error: " << buf;
//        Log( os.str() );
//        return "";
//    }
//*/
//
//    ifstream fs( static_cast<wchar_t*>( path ) );
//    if ( !fs )
//    {
//        Log( "Failed to open iTunes XML file" );
//        return "";
//    }
//    
//    // OK, we have the XML file ready and willing
//    
//
//}

#endif // WIN32

