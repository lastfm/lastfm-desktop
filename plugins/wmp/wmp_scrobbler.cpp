/////////////////////////////////////////////////////////////////////////////
//
// wmp_scrobbler.cpp : Implementation of CWmp_scrobbler
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "wmp_scrobbler.h"
#include "EncodingUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler::CWmp_scrobbler
// Constructor

CWmp_scrobbler::CWmp_scrobbler()
               :m_hWndParent( NULL )
{
    m_dwAdviseCookie = 0;
	m_bNewSong = FALSE;
	lstrcpyn(m_szPluginText, AUDIOSCROBBLER_APPNAME, sizeof(m_szPluginText) / sizeof(m_szPluginText[0]));
}

/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler::~CWmp_scrobbler
// Destructor

CWmp_scrobbler::~CWmp_scrobbler()
{
}

/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler:::FinalConstruct
// Called when an plugin is first loaded. Use this function to do one-time
// intializations that could fail instead of doing this in the constructor,
// which cannot return an error.

HRESULT CWmp_scrobbler::FinalConstruct()
{
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler:::FinalRelease
// Called when a plugin is unloaded. Use this function to free any
// resources allocated in FinalConstruct.

void CWmp_scrobbler::FinalRelease()
{		
	StopScrobbling();
    ReleaseCore();
}

/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler::SetCore
// Set WMP core interface

HRESULT CWmp_scrobbler::SetCore(IWMPCore *pCore)
{
    HRESULT hr = S_OK;

    // release any existing WMP core interfaces
    ReleaseCore();

    // If we get passed a NULL core, this  means
    // that the plugin is being shutdown.

    if (pCore == NULL)
    {
        return S_OK;
    }

    m_spCore = pCore;

    // connect up the event interface
    CComPtr<IConnectionPointContainer>  spConnectionContainer;

    hr = m_spCore->QueryInterface( &spConnectionContainer );

    if (SUCCEEDED(hr))
    {
        hr = spConnectionContainer->FindConnectionPoint( __uuidof(IWMPEvents), &m_spConnectionPoint );
    }

    if (SUCCEEDED(hr))
    {
        hr = m_spConnectionPoint->Advise( GetUnknown(), &m_dwAdviseCookie );

        if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
        {
            m_spConnectionPoint = NULL;
        }
    }

    m_wmpBootStrap.setParentHwnd( FindWindow( "WMPlayerApp", "Windows Media Player" ) );
    char filename[512];

	// Get the DLL filename etc
	GetModuleFileName( _Module.GetModuleInstance(), filename, sizeof( filename ) );
	CStdString Temp = filename;
	int pos = Temp.ReverseFind('\\');
	
	StartScrobbling( Temp.Left( pos + 1 ), _Module.GetModuleInstance() );
    
    if( m_wmpBootStrap.bootStrapRequired() )
    {
        m_wmpBootStrap.setCore( m_spCore );
        m_wmpBootStrap.setScrobSub( &m_Submitter );
        m_wmpBootStrap.setModuleHandle( _Module.GetModuleInstance() );
        m_wmpBootStrap.startBootStrap();
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler::ReleaseCore
// Release WMP core interfaces

void CWmp_scrobbler::ReleaseCore()
{
    if (m_spConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
        {
            m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
            m_dwAdviseCookie = 0;
        }
        m_spConnectionPoint = NULL;
    }

    if (m_spCore)
    {
        m_spCore = NULL;
    }
}




/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler::DisplayPropertyPage
// Display property page for plugin

HRESULT CWmp_scrobbler::DisplayPropertyPage(HWND hwndParent)
{
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler::GetProperty
// Get plugin property

HRESULT CWmp_scrobbler::GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty)
{
    if (NULL == pvarProperty)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CWmp_scrobbler::SetProperty
// Set plugin property

HRESULT CWmp_scrobbler::SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty)
{
    return E_NOTIMPL;
}

CStdString	CWmp_scrobbler::GetPlayerVersion()
{
	USES_CONVERSION;
	CStdString strVersion;
	CComBSTR bstrVersion;

	if(m_spCore)
	{
        bstrVersion.Empty();
		m_spCore->get_versionInfo(&bstrVersion);
		
		strVersion = "Windows Media Player ";
		strVersion += W2A(bstrVersion);
	}

	return strVersion;
}

BOOL CWmp_scrobbler::IsPaused()
{
	WMPPlayState state;

	if(m_spCore)
	{
		m_spCore->get_playState(&state);

		if(state == wmppsPaused)
			return TRUE;
	}

	return FALSE;
}

BOOL CWmp_scrobbler::IsStopped()
{
	WMPPlayState state;

	if(m_spCore)
	{
		m_spCore->get_playState(&state);

		if(state == wmppsStopped ||
		   state == wmppsReady ||
           state == wmppsMediaEnded)
        {
			return TRUE;
	    }
	}

	return FALSE;
}

BOOL CWmp_scrobbler::IsPlaying()
{
	WMPPlayState state;

	if(m_spCore)
	{
		m_spCore->get_playState(&state);

		if(state == wmppsPlaying)
			return TRUE;
	}

	return FALSE;
}

CStdString CWmp_scrobbler::GetCurrentSongFileName()
{
	USES_CONVERSION;
	CStdString strSong;
	CComPtr<IWMPMedia> pMedia;
	BSTR bstrSong;

	if(m_spCore)
	{
		m_spCore->get_currentMedia(&pMedia);
		
		if(pMedia != NULL)
		{
			pMedia->get_sourceURL(&bstrSong);

            char buf[1000];
            EncodingUtils::UnicodeToUtf8(
                bstrSong, -1,
                buf, 999);
            
            strSong = buf;
        }
	}

	return strSong;
}

int CWmp_scrobbler::GetTrackLength()
{
	double dDuration = -1;

	CComPtr<IWMPMedia> pMedia;
	
	if(m_spCore)
	{
		m_spCore->get_currentMedia(&pMedia);
		
		if(pMedia != NULL)
		{
			pMedia->get_duration(&dDuration);
		}
	}

	return (int)dDuration;
}

int CWmp_scrobbler::GetTrackPosition()
{
	double dPosition = -1;

	CComPtr<IWMPControls> pControls;
	
	if(m_spCore)
	{
		m_spCore->get_controls(&pControls);
		
		if(pControls != NULL)
		{
			pControls->get_currentPosition(&dPosition);
		}
	}

	return (int)dPosition;
}

BOOL CWmp_scrobbler::GetCurrentSong(SONG_INFO* pSong)
{
	USES_CONVERSION;
	CComPtr<IWMPMedia> pMedia;
	CComBSTR bstrValue;
	//BSTR bstrName;
	BOOL bRet = FALSE;
	long lCount = 0;
	
	if(	(m_spCore) &&
		(pSong != NULL))
	{
		m_spCore->get_currentMedia(&pMedia);
		
		if(pMedia != NULL)
		{
			pMedia->get_attributeCount(&lCount);

/*
			for(long i = 0; i <= lCount; i++)
			{
				pMedia->getAttributeName(i, &bstrName);
				bstrValue.Empty();
				pMedia->getItemInfo(bstrName, &bstrValue);
				PRINTF(DEBUG_INFO, "WMPMEDIA", "Attribute %s - value %s", W2A(bstrName), W2A(bstrValue));
			}
            // Better check for local/streaming for the future
			bstrValue.Empty();
    		pMedia->getItemInfo(L"type", &bstrValue);
*/    		

            bstrValue.Empty();
			pMedia->getItemInfo(L"Author", &bstrValue);
            EncodingUtils::UnicodeToUtf8(bstrValue, -1, pSong->m_strArtist, SONG_INFO_FIELD_SIZE);
            bstrValue.Empty();
            pMedia->getItemInfo(L"Title", &bstrValue);
            EncodingUtils::UnicodeToUtf8(bstrValue, -1, pSong->m_strTrack, SONG_INFO_FIELD_SIZE);
            bstrValue.Empty();
			pMedia->getItemInfo(L"Album", &bstrValue);
            EncodingUtils::UnicodeToUtf8(bstrValue, -1, pSong->m_strAlbum, SONG_INFO_FIELD_SIZE);
            bstrValue.Empty();
			pMedia->getItemInfo(L"Genre", &bstrValue);
            EncodingUtils::UnicodeToUtf8(bstrValue, -1, pSong->m_strGenre, SONG_INFO_FIELD_SIZE);
            bstrValue.Empty();
			pMedia->getItemInfo(L"Comment", &bstrValue);
            EncodingUtils::UnicodeToUtf8(bstrValue, -1, pSong->m_strComment, SONG_INFO_FIELD_SIZE);
            bstrValue.Empty();
			pMedia->getItemInfo(L"SourceURL", &bstrValue);
            EncodingUtils::UnicodeToUtf8(bstrValue, -1, pSong->m_strFileName, SONG_INFO_FIELD_SIZE);
			
            bstrValue.Empty();
			pMedia->getItemInfo(L"Duration", &bstrValue);
			pSong->m_nLength = atoi(W2A(bstrValue));
            bstrValue.Empty();
			pMedia->getItemInfo(L"Track", &bstrValue);
			pSong->m_nTrackNo = atoi(W2A(bstrValue));
            bstrValue.Empty();
			pMedia->getItemInfo(L"Year", &bstrValue);
			pSong->m_nYear = atoi(W2A(bstrValue));
			
			bRet = TRUE;
		}	
	}

	return bRet;
}