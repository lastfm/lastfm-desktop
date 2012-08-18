#include "stdafx.h"
#include "wmpBootStrap.h"
#include "wmp_scrobbler.h"

#define TEST_OR_CONTINUE( function ) if( function != S_OK ) \
    continue

enum BootStrapDetails{ ARTIST=0, ALBUM, TRACK, DURATION, PLAYCOUNT, FILENAME, TIMESTAMP };

wmpBootStrap::wmpBootStrap( )
             :BootStrap( L"wmp" ),
              m_core( NULL )
{
    //Extract the following data
    attributes[L"Artist"]             = ARTIST;
    attributes[L"Album"]              = ALBUM;
    attributes[L"Title"]              = TRACK;
    attributes[L"Duration"]           = DURATION;
    attributes[L"UserPlayCount"]      = PLAYCOUNT;
    attributes[L"SourceUrl"]          = FILENAME;
    attributes[L"UserLastPlayedTime"] = TIMESTAMP;
}

wmpBootStrap::~wmpBootStrap(void)
{
}

bool wmpBootStrap::start()
{

    ASSERT( m_core );
    
    #define TEST_OR_RETURN( function ) if( function != S_OK ) \
                                         return false
    CComPtr<IWMPMediaCollection> mediaCollection;

    TEST_OR_RETURN( m_core->get_mediaCollection( &mediaCollection ) );

    CComPtr<IWMPPlaylist> allMediaPlaylist;
    
    TEST_OR_RETURN( mediaCollection->getAll( &allMediaPlaylist ) );

    long mediaCount;
    
    TEST_OR_RETURN( allMediaPlaylist->get_count( &mediaCount ) );

    CComPtr<IWMPMedia> currentMedia;
        
    setProgressRange( mediaCount );

    for ( long mediaIndex = 0; mediaIndex < mediaCount; mediaIndex++ )
    {
        //Update progress bar
        setProgressValue( mediaIndex + 1 );

        while( m_paused){
            WaitForSingleObject( m_pauseStateChanged, INFINITE );
            if(m_cancel) return false;
        }

        TEST_OR_CONTINUE( allMediaPlaylist->get_item( mediaIndex, &currentMedia ) );
        
        //Get media type
        _bstr_t mediaTypeValue;
        _bstr_t mediaTypeAttrib = _T( "MediaType" );
        TEST_OR_CONTINUE( currentMedia->getItemInfo( mediaTypeAttrib.GetBSTR() , &mediaTypeValue.GetBSTR() ) );

        //Skip any media that is not audio
        std::wstring mediaTypeValueString = mediaTypeValue;
        if( mediaTypeValueString != L"audio" )
            continue;
            
        readAttributes( currentMedia );

    }
    
    completed();


    #undef TEST_OR_RETURN

    return true;
}

void wmpBootStrap::setCore(IWMPCore *core)
{
    m_core = core;
}

void wmpBootStrap::readAttributes( IWMPMedia* media )
{
    BootStrapItem details;

    //iterate through each attribute
    for( std::map<_bstr_t, BootStrapDetails>::iterator iter = attributes.begin(); iter != attributes.end(); iter++ )
    {
        _bstr_t value;
        BSTR attribute = iter->first;
        TEST_OR_CONTINUE( media->getItemInfo( attribute , &value.GetBSTR() ) );
        
        //escape invalid xml characters
        std::wstring valueStr = value;
        
        switch( iter->second )
        {
            case ARTIST:
                details.artist = valueStr;
                break;
                
            case ALBUM:
                details.album = valueStr;
                break;
                
            case TRACK:
                details.track = valueStr;
                break;
                
            case DURATION:
                details.duration = valueStr;
                break;
                
            case PLAYCOUNT:
                details.playcount = valueStr;
                break;
                
            case FILENAME:
                details.filename = valueStr;
                break;
                
            case TIMESTAMP:
                //TODO: convert string timestamp into epochs.
                //      this requires checking user's default locale
                break;
        }
    }
    appendTrack( &details );
}


void wmpBootStrap::getDateTimeFormat()
{
    char dateFormat[80];
    char timeFormat[80];
    ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, dateFormat, 80 );
    ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, timeFormat, 80 );
    m_dateFormat << dateFormat;
    m_timeFormat << timeFormat;
}

#undef TEST_OR_CONTINUE