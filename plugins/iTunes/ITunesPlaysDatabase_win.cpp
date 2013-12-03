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

#include "ITunesTrack.h"
#include "common/c++/logger.h"

#include "sqlite3.h"


bool
ITunesPlaysDatabase::sync( const ExtendedITunesTrack& track )
{
    if ( track.isNull() )
    {
        // This means the track has no COM pointer or no path
        return false;
    }

    try
    {
        int const diff = track.playCountDifference();
        LOGWL( 3, "Diff = " << diff << " for " << track.toString() );

        int const twiddlyPlayCount = playCount( track );

        // if the track is new and not yet in the database then twiddlyPlayCount
        // will be -1, therefore sync with the iTunes playCount for this track
        int const playCountToSync = (twiddlyPlayCount == -1)
                ? track.playCount()
                : twiddlyPlayCount + diff;

        if ( playCountToSync == twiddlyPlayCount )
        {
            // Nothing to do. This isn't really an error, we just happened to sync
            // even though the playcount hadn't inced.
            return true;
        }

        const char* format = "REPLACE INTO " TABLE_NAME " ( persistent_id, play_count ) VALUES ( '%q', '%d' );";
        char* token = sqlite3_mprintf( format,
                                       Moose::wStringToUtf8( track.persistentId() ).c_str(),
                                       playCountToSync );

        LOGWL( 3, "Syncing local db for: " << track.artist() << " - " << track.track() );

        bool success = query( token );
        sqlite3_free( token );

        return success;
    }
    catch ( ITunesException& )
    {
        // This means COM failed to retrieve the playcount, abort
        // NOTE no logging because a higher up function logs already
        return false;   
    }
}

bool
ITunesPlaysDatabase::syncOld( const ExtendedITunesTrack& track )
{
    if ( track.isNull() )
    {
        // This means the track has no COM pointer or no path
        return false;
    }

    try
    {
        int const diff = track.playCountDifference();
        LOGWL( 3, "Diff = " << diff << " for " << track.toString() );

        int const twiddlyPlayCount = playCountOld( track );

        // if the track is new and not yet in the database then twiddlyPlayCount
        // will be -1, therefore sync with the iTunes playCount for this track
        int const playCountToSync = (twiddlyPlayCount == -1)
                ? track.playCount()
                : twiddlyPlayCount + diff;

        if ( playCountToSync == twiddlyPlayCount )
        {
            // Nothing to do. This isn't really an error, we just happened to sync
            // even though the playcount hadn't inced.
            return true;
        }

        const char* format = "REPLACE INTO itunes_db ( persistent_id, path, play_count ) VALUES ( '%q', '%q', '%d' );";
        char* token = sqlite3_mprintf( format,
                                       Moose::wStringToUtf8( track.persistentId() ).c_str(),
                                       Moose::wStringToUtf8( track.path() ).c_str(),
                                       playCountToSync );

        LOGWL( 3, "Syncing old local db for: " << track.artist() << " - " << track.track() );

        bool success = query( token );
        sqlite3_free( token );

        return success;
    }
    catch ( ITunesException& )
    {
        // This means COM failed to retrieve the playcount, abort
        // NOTE no logging because a higher up function logs already
        return false;   
    }
}
