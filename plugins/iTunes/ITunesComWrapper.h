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
#ifndef ITUNESCOMWRAPPER_H
#define ITUNESCOMWRAPPER_H

#ifdef WIN32

#include "ITunesTrack.h"
#include "ITunesEventInterface.h"

// Needed for CoInitializeEx
#ifndef _WIN32_DCOM 
    #define _WIN32_DCOM 
#endif    

#include <windows.h>

// Must include objbase before iTunes interface or it won't compile
#include <objbase.h>
#include <atlbase.h>
#include <atlsafe.h>
#include <atlcom.h>
#include "lib/3rdparty/iTunesCOMAPI/iTunesCOMInterface.h"

#include <string>
#include <vector>

class ITunesComEventSink;

/** @author Erik Jalevik <erik@last.fm>
  * @brief Uses COM to query iTunes for track information.
  *        All functions that access COM directly will throw an ITunesException
  *        if something went wrong.
  */
class ITunesComWrapper
{
public:

    /// If you want events, pass in an implementation of the event handler interface,
    /// leave it as 0 and the event sink won't get initialised.
    ITunesComWrapper( ITunesEventInterface* handler = 0 );
    ~ITunesComWrapper();

    /// If this returns false, we didn't manage to initialise COM and all the
    /// other malarkey required for this to work.
    bool isValid() { return m_iTunesApp != 0 && m_trackCount != -1 && m_allTracks != 0; }

    /// Call libraryTrackCount first...
    long libraryTrackCount();

    /// if you call this instead of the libraryTrackCount() equivalent
    /// all other functions that involve m_allTracks will reference the iPod
    /// instead FIXME sucks!
    /// NOTE as above, call this before calling track()
    long iPodLibraryTrackCount();

    /// Then pass an index in the range 0..libraryTrackCount to this function
    ITunesTrack track( long idx );

    /// Or an ITunesIdSet to this. An ITunesIdSet does not necessarily represent
    /// a track, it could be any iTunes object. If the set of IDs passed in does
    /// not represent a track, an ITunesException will be thrown.
    ITunesTrack track( const ITunesEventInterface::ITunesIdSet& ids );

    /// Get currently playing track in iTunes
    ITunesTrack currentTrack();

    /// Get state of player (stopped/running etc)
    ITPlayerState playerState();
    
    /// Get player position in seconds
    long playerPosition();

    /// Get iTunes version
    std::wstring iTunesVersion();

    /// Search library. ITPlaylistSearchField is an iTunes type.
    std::vector<ITunesTrack>
    searchForTrack( ITPlaylistSearchField searchField, std::wstring searchTerm );

private:
    void initialiseCom();
    void uninitialiseCom();

    void setPersistentIDForTrack( ITunesTrack& t, IITTrack* track );

    /// Connects iTunes events to our event sink
    void connectSink( ITunesEventInterface* handler );

    /// Throws if no iPod found in iTunes
    IITPlaylist* iPodLibraryPlaylist();

    static std::wstring bstrToWString( BSTR bstr );
    static bool handleComResult( HRESULT res, std::wstring err = L"" );
    static void logComError( HRESULT res, std::wstring err );

    IiTunes* m_iTunesApp;
    IITTrackCollection* m_allTracks;
    long m_trackCount;

    CComObject<ITunesComEventSink>* m_sink;
    DWORD m_sinkId; // needed to close the connection
    IUnknown* m_sinkAsUnknown;
    
    friend ITunesTrack;
};


/** @author Erik Jalevik <erik@last.fm>
  * @brief Internal handler for COM events fired by iTunes.
  */
class ATL_NO_VTABLE ITunesComEventSink : 
    public CComObjectRootEx<CComObjectThreadModel>,
    public IDispatch
{
public:

    BEGIN_COM_MAP(ITunesComEventSink)
        COM_INTERFACE_ENTRY( IDispatch )
        COM_INTERFACE_ENTRY_IID( DIID__IiTunesEvents, IDispatch )
    END_COM_MAP()

    ITunesComEventSink() : m_handler( 0 ) { }

    /// You must call this function with a valid handler after initialising this class.
    void setHandler( ITunesEventInterface* handler ) { m_handler = handler; }

    // IDispatch stuff
    HRESULT __stdcall Invoke( DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                              DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr );
    HRESULT __stdcall GetTypeInfoCount( UINT* ) { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo( UINT, LCID, ITypeInfo** ) { return E_NOTIMPL; }
    HRESULT __stdcall GetIDsOfNames( REFIID, LPOLESTR*, UINT, LCID, DISPID* ) { return E_NOTIMPL; }

private:

    ITunesEventInterface* m_handler;

    void onDatabaseChangedEvent( VARIANT deletedObjectIDs, VARIANT changedObjectIDs );

    ITunesEventInterface::ITunesIdSet
    getIdsFromSafeArray( long index, CComSafeArray<VARIANT>& array );

};


// We need one of these things hanging around for the ATL calls to work.
// It's initialised in connectSink.
class TheAtlModule : public CAtlDllModuleT<TheAtlModule> { };
//class TheAtlModule : public CAtlExeModuleT<TheAtlModule> { };


#endif // WIN32

#endif // ITUNESCOMTHREAD_H