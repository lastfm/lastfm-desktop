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
#ifndef ITUNES_TRACK_H
#define ITUNES_TRACK_H

#include "ITunesExceptions.h"

#ifdef WIN32
    // Disable warning about exception specifications
    #pragma warning( disable : 4290 )

	struct IITTrack;
#endif

#include <string>


/** @author Max Howell <max@last.fm> -- mac
  * @author Erik Jalevik <erik@last.fm> -- windows
  * @brief Uses AppleScript or COM to retrieve properties of an iTunes track.
  *        All functions that access COM directly will throw an ITunesException
  *        if something went wrong.
  */
class ITunesTrack
{
public:
  #ifdef WIN32

    ITunesTrack(); // creates a null track
      
    /** Creates a track from a COM track object. Ownership is passed to this
      * class and it will call Release on the object when done. Never throws. */
    ITunesTrack( IITTrack* track );

    // Copy ctor (needed because of owned COM pointer)
    ITunesTrack( const ITunesTrack& that );
    ITunesTrack& operator=( const ITunesTrack& that );
    
    ITunesTrack::~ITunesTrack();

    /** Unicode */
    std::wstring track() const;
    std::wstring artist() const;
	std::wstring albumArtist() const;
    std::wstring album() const;

    /** Date format: "YYYY-MM-DD HH:MM:SS", returns an empty string if track was never played */
    std::wstring lastPlayed() const;

    /** In seconds */
    long duration() const;
    
    /** Convenience function for logging etc */
    std::wstring toString() const { return artist() + L" - " + track(); }

    /** Uses runtime ID to compare the two. */
    bool isSameAs( const ITunesTrack& that );

    /** These two fields are always populated when an iTunes track is created */
    std::wstring persistentId() const { return m_id; }
    std::wstring path() const { return m_path; }

    void setPersistentId( std::wstring id ) { m_id = id; }

    /** podcasts have the option of not scrobbling and videos are never scrobbled */
    bool podcast();
    bool video();

  #else
      
    /** These two fields are always populated when an iTunes track is created */
    std::string persistentId() const { return m_id; }
    std::string path() const { return m_path; }
    std::string kind() const { return m_kind; }
  
  #endif

    /** The below fields are real-time */
    long playCount() const throw( PlayCountException );
    bool isNull() const;

protected:

  #ifdef WIN32

    std::wstring m_id;
    std::wstring m_path;

    std::wstring pathForTrack( IITTrack* track );
    void clone( const ITunesTrack& that );

    IITTrack* m_comTrack;

  #else

    std::string m_id; // utf8
    std::string m_path; // utf8
    std::string m_dbid; //database ID
    std::string m_kind; // "Audio CD Track" etc
    
    /** executes an applescript in the bundle and returns the output */
    static std::string scriptResult( const char* filename, const std::string& argv1 = "" ) throw();

  #endif  
};


/** @author Max Howell <max@last.fm>
  * @brief Specialised variety of ITunesTrack, used for playCount sync 
  *   operations, where we need the difference in playCount since the object was
  *   initialised */
class ExtendedITunesTrack : public ITunesTrack
{
public:
    ExtendedITunesTrack() : m_initialPlayCount( -2 )
    {}
    
#ifdef WIN32
    static ExtendedITunesTrack from( const ITunesTrack& that )
    {
		ExtendedITunesTrack t;
		t.ITunesTrack::operator=( that );
        t.m_initialPlayCount = that.playCount();
        return t;
    }
#else
    /** returns the track currently playing in iTunes */
    static ExtendedITunesTrack currentTrack();
#endif

    int playCountDifference() const throw( PlayCountException );
    int initialPlayCount() const { return m_initialPlayCount; }
    void setInitialPlaycount( int pc ) { m_initialPlayCount = pc; }
    
private:
    int m_initialPlayCount;
};

#endif // ITUNESTRACK_H
