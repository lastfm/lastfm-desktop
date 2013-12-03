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
#ifndef ITUNES_LIBRARY_TRACK_H
#define ITUNES_LIBRARY_TRACK_H

#ifdef WIN32

#include <lastfm/Track.h>
#include "plugins/iTunes/ITunesTrack.h"
#include <QSharedData>
#include <QSharedDataPointer>

namespace COM
{
    using ::ITunesTrack;
}


/** @author <max@last.fm>
  *
  * We are QSharedData to use polymorphism but still allow
  * ITunesLibraryTracks to be passed by value.
  *
  * It would be nicer to derive ITunesTrack, but ITunesComWrapper is not
  * designed for it, and ITunesTrack is not virtual enough, so we have this
  * mess instead 
  */
struct ITunesLibraryTrackData : public QSharedData
{
    ITunesLibraryTrackData( const ITunesTrack& t ) : i( t )
    {}

    virtual ~ITunesLibraryTrackData()
    {}

    /** the path is the uniqueId for iTunes library tracks on Windows */
    virtual QString uniqueId() const { return QString::fromStdWString( i.persistentId() ); }

    /** @returns false if the track doesn't exist in the iTunes Library */
    virtual bool isNull() const { return i.isNull(); }

    /** @returns -1 if COM failure or isNull() */
    int playCount() const { return i.playCount(); }

    COM::ITunesTrack i;
};


/** @author <max@last.fm> 
  * @brief represents a track from an iPod source
  *
  * NOTE only used for manual iPod scrobbling, since iPods have no path, this
  * is the only unique ID we have available :(
  */
struct IPodLibraryTrackData : public ITunesLibraryTrackData
{
    IPodLibraryTrackData( const ITunesTrack& t ) : ITunesLibraryTrackData( t )
    {}

    /** isNull from COM::ITunesTrack returns true if m_path is null too *
    * so we can't rely on that at all :( */
    virtual bool isNull() const { return uniqueId().remove( '\t' ).isEmpty(); }

    /** iTunes returns empty paths for tracks stored on the iPod */
    virtual QString uniqueId() const
    {
        return QString::fromStdWString( i.artist() + L'\t' + i.track() + L'\t' + i.album() );
    }
};


    /** @author <max@last.fm> 
      */
    class ITunesLibraryTrack
    {
        friend class ITunesLibrary;
        
        ITunesLibraryTrack()
        {}

        QSharedDataPointer<ITunesLibraryTrackData> d;

    public:
        bool isNull()      const { return !d || d->isNull(); }
        QString uniqueId() const { Q_ASSERT( d ); return d->uniqueId(); }
        QString persistentId() const { return uniqueId(); }
        int playCount()    const { Q_ASSERT( d ); return d->playCount(); }

        /** @returns a TrackInfo object filled out with minimal information,
          * check to see if  what you want is assigned before assuming so!
          * It gets the data from iTunes using AppleScript/COM 
          *
          * @defined ITunesLibrary.cpp
          */
        lastfm::Track lastfmTrack() const;
    };


#else //MAC
    #include <lastfm/Track.h>
    #include "PlayCountsDatabase.h"
    
    template <typename T> class QList;
    
    /** @author <max@last.fm>
      * private because there is no "isA" relationship 
      */
    class ITunesLibraryTrack : private PlayCountsDatabase::Track
    {
        friend class ITunesLibrary;
        friend class QList<ITunesLibraryTrack>;
        
        ITunesLibraryTrack() // for QList only
        {}
        
    public:
        ITunesLibraryTrack( const QString& uid, int c ) : PlayCountsDatabase::Track( uid, c )
        {}
        
        lastfm::Track lastfmTrack() const;

        using PlayCountsDatabase::Track::isNull;
        using PlayCountsDatabase::Track::uniqueId;
        using PlayCountsDatabase::Track::playCount;

        QString persistentId() const { return uniqueId(); }

    private:
        /** the persistent ID of the source for this track, if empty, we use
          * the default iTunes library */
        QString m_sourcePersistentId;
    };
#endif

#endif
