/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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

#include <QDebug>
#include <QStringList>
#include <QUrl>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "plugins/iTunes/ITunesComWrapper.h"

#include "PlayerCommandParser.h"

using std::invalid_argument;

PlayerCommandParser::PlayerCommandParser( QString line ) throw( std::invalid_argument )
{
    line = line.trimmed();
    if (line.isEmpty()) throw invalid_argument( "Command string seems to be empty" );

    qDebug() << line;

    m_command = extractCommand( line ); //removes the command string from line

    line = line.trimmed();

    QMap<QChar, QString> const args = extractArgs( line );
    QString const required = requiredArgs( m_command );

    for (int i = 0; i < required.length(); ++i)
    {
        QChar const c = required[i];
        if (!args.contains( c ))
            throw invalid_argument( std::string( "Mandatory argument unspecified: " ) + c.toLatin1() );
    }

    m_playerId = args['c'];
    
    if (m_playerId.isEmpty())
        throw invalid_argument( "Player ID cannot be zero length" );

    switch (m_command)
    {
        case CommandStart:
            m_track = extractTrack( args );
            break;
        case CommandBootstrap:
            m_username = args['u'];
            break;
        case CommandInit:
            m_applicationPath = args['f'];
        default:
            break;
    }
}


PlayerCommand
PlayerCommandParser::extractCommand( QString& line )
{
    int const n = line.indexOf( ' ' );
    if (n == -1) throw invalid_argument( "Unable to parse" );

    QString const command = line.left( n ).toUpper();

    // Trim off command from passed in string
    line = line.mid( n + 1 );

    if (command == "START") return CommandStart;
    if (command == "STOP") return CommandStop;
    if (command == "PAUSE") return CommandPause;
    if (command == "RESUME") return CommandResume;
    if (command == "BOOTSTRAP") return CommandBootstrap;
    if (command == "INIT") return CommandInit;
    if (command == "TERM") return CommandTerm;

    throw invalid_argument( "Invalid command" );
}


namespace mxcl
{
    /** replaces '&&' and splits on remaining single '&' */
    QStringList static inline split( QString line )
    {
        QStringList parts;
        int start = 0, i = 0;
        int end = 0;
        while ((end = line.indexOf( '&', i )) != -1)
        {
            i = end + 1;
            if (line[i] == '&') {
                line.remove( end, 1 ); //convert && to &
                continue;
            }
            parts += line.mid( start, end - start );
            start = i;
        }
        return parts << line.mid( start );
    }
}


struct Pair
{
    Pair( const QString& s )
    {
        if (s[1] == '=')
        {
            key = s[0];
            value = s.mid( 2 );
        }
    }
    
    QChar key;
    QString value;
};


QMap<QChar, QString>
PlayerCommandParser::extractArgs( const QString& line )
{
    QMap<QChar, QString> map;

    // split by single & only, doubles are in fact &
    foreach (Pair pair, mxcl::split( line ))
    {
        if (pair.key == QChar()) 
            throw invalid_argument( std::string( "Invalid pair: " ) + pair.key.toLatin1() + '=' + std::string(pair.value.toUtf8().data()) );

        if (map.contains( pair.key ))
            throw invalid_argument( std::string( "Field identifier occurred twice in request: " ) + pair.key.toLatin1() );

        map[pair.key] = pair.value.trimmed();
    }

    return map;
}


QString
PlayerCommandParser::requiredArgs( PlayerCommand c )
{
    switch (c)
    {   
        case CommandStart: 
            return "catblp";
        case CommandBootstrap:
            return "cu";
        case CommandInit:
            return "cf";
        case CommandStop:
        case CommandPause:
        case CommandResume:
        case CommandTerm:
        default: // gcc 4.2 is stupid
            return "c";
    }
}


Track
PlayerCommandParser::extractTrack( const QMap<QChar, QString>& args )
{
    lastfm::MutableTrack track;
    track.setArtist( args['a'] );
    track.setAlbumArtist( args['d'] );
    track.setTitle( args['t'] );
    track.setAlbum( args['b'] );
    track.setMbid( Mbid( args['m'] ) );
    track.setDuration( args['l'].toInt() );
    track.setUrl( QUrl::fromLocalFile( QUrl::fromPercentEncoding( args['p'].toUtf8() ) ) );
    track.setSource( Track::Player );
    track.setExtra( "playerId", args['c'] );
    track.setExtra( "playerName", playerName() );

#ifdef Q_OS_WIN

    if ( args['c'] == "itw" )
    {
        ITunesComWrapper* com = new ITunesComWrapper;
        ITunesTrack comTrack = com->currentTrack();
        bool podcast = comTrack.podcast();
        bool video = comTrack.video();
        QUrl path = QUrl::fromLocalFile( QString::fromStdWString( comTrack.path() ) );

        qDebug() << QString::fromStdWString( comTrack.artist() )
                 << QString::fromStdWString( comTrack.track() )
                 << podcast
                 << video
                 << path;

        track.setUrl( path );
        track.setPodcast( podcast );
        track.setVideo( video );
        delete com;
    }

#endif

    //TODO should be done earlier, NOTE don't get the plugin to send a stamp 
    // time as this is prolly unecessary, and I bet you get new bugs!
    track.stamp();
    
    return track;
}
