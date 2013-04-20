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

#include "ProgressBar.h"

#include "lib/unicorn/UnicornSettings.h"

#include "../Services/RadioService.h"
#include "../Services/ScrobbleService.h"
#include "../Services/ScrobbleService/StopWatch.h"

#include <lastfm/Track.h>
#include <QWidget>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

#include <phonon/MediaObject>


ProgressBar::ProgressBar( QWidget* parent )
    :QFrame( parent )
{
    m_scrobbleMarkerOn = QImage(":/scrobble_marker_ON.png");
    m_scrobbleMarkerOff = QImage(":/scrobble_marker_OFF.png");
}


void
ProgressBar::setTrack( const Track& track )
{
    m_track = track;
    m_frame = 0;
}


void
ProgressBar::onFrameChanged( int frame )
{
    m_frame = frame;
    update();
}


void
ProgressBar::resizeEvent( QResizeEvent* e )
{
    // we set theick interval so we are only told
    // when the progress goes to the next pixel
    if ( !m_track.isNull() && RadioService::instance().mediaObject() )
        RadioService::instance().mediaObject()->setTickInterval( ( m_track.duration() * 1000 ) / e->size().width()  );
}

void
ProgressBar::paintEvent( QPaintEvent* e )
{
    QFrame::paintEvent( e );

    QPainter p( this );

    StopWatch* sw = ScrobbleService::instance().stopWatch();

    if ( m_track != Track() )
    {
        QFont timeFont = font();
        timeFont.setPixelSize( 10 );
        setFont( timeFont );

        p.setPen( QColor( 0x333333 ) );

        if ( m_track.extra( "playerId" ) != "spt" && m_track.extra( "playerId" ) != "mpris2" )
        {
            if ( m_track.duration() >= 30 )
            {
                QString format( "m:ss" );

                QTime duration( 0, 0 );
                duration = duration.addMSecs( m_track.duration() * 1000 );
                QTime progress( 0, 0 );
                progress = progress.addMSecs( m_frame );

                if ( duration.hour() > 0 )
                    format = "h:mm:ss";

                QTextOption timeTextOption;
                timeTextOption.setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

                QString timeText;

                if ( m_track.source() == Track::LastFmRadio )
                    timeText = QString( "%1 / %2" ).arg( progress.toString( format ), duration.toString( format ) );
                else
                    timeText = QString( "%1" ).arg( progress.toString( format ) );

                p.setPen( QColor( 0x333333 ) );
                p.drawText( rect().adjusted( 6, 0, 0, 0 ), timeText, timeTextOption );

                QFontMetrics fm( font() );
                int indent = fm.width( timeText ) + ( 2 * 6 ) + 2;

                int width = this->width() - indent;

                p.setPen( QColor( 0xbdbdbd ));
                p.drawLine( QPoint( indent - 2, rect().top() ),
                            QPoint( indent - 2, rect().bottom() - 1 ) );

                p.setPen( QColor( 0xe6e6e6 ) );
                p.drawLine( QPoint( indent - 1, rect().top() ),
                            QPoint( indent - 1, rect().bottom() - 1 ) );


                // draw the chunk
                p.setPen( Qt::transparent );
                p.setBrush( m_chunk );
                p.drawRect( rect().adjusted( indent, 0, ((m_frame * width) / (m_track.duration() * 1000)) - width, -1) );

                //bool scrobblingOn = unicorn::UserSettings().value( "scrobblingOn", true ).toBool();
                bool scrobblingOn = ScrobbleService::instance().scrobblableTrack( m_track );

                if ( scrobblingOn ||
                     ( !scrobblingOn && (m_track.scrobbleStatus() != Track::Null) ) )
                {
                    if ( !scrobblingOn && m_track.scrobbleStatus() != Track::Null )
                    {
                        QTextOption textOption;
                        textOption.setAlignment( Qt::AlignVCenter | Qt::AlignRight );
                        p.drawText( rect().adjusted( 0, 0, -6, 0 ), tr( "Scrobbling off" ), textOption );
                    }

                    uint scrobblePoint = sw->scrobblePoint() * 1000;

                    int scrobbleMarker = indent + (scrobblePoint * width) / ( m_track.duration() * 1000 );

                    p.setPen( QPen( QColor( 0xbdbdbd ), 1, Qt::DashLine) );
                    p.drawLine( QPoint( scrobbleMarker - 1, rect().top() ),
                                QPoint( scrobbleMarker - 1, rect().bottom() ) );

                    p.setPen( QPen( QColor( 0xe6e6e6 ), 1, Qt::DashLine) );
                    p.drawLine( QPoint( scrobbleMarker, rect().top() ),
                                QPoint( scrobbleMarker, rect().bottom() ) );

                    // Draw the 'as'!
                    // if the scrobble marker is too close to the left draw the 'as' on the right hand side
                    QPoint asPoint;
                    QImage as( m_track.scrobbleStatus() != Track::Null ? m_scrobbleMarkerOn : m_scrobbleMarkerOff );

                    if ( ( as.width() + 10 ) > scrobbleMarker - indent )
                        asPoint = QPoint ( scrobbleMarker + 5 , (rect().height() / 2) - (as.height() / 2) );
                    else
                        asPoint = QPoint ( scrobbleMarker - as.width() - 5, (rect().height() / 2) - (as.height() / 2) );

                    p.drawImage( asPoint, as );

                }
                else
                {
                    QString offMessage = NULL;

                    if ( unicorn::UserSettings().value( "scrobblingOn", true ).toBool() )
                    {
                        if ( ScrobbleService::instance().isDirExcluded( m_track ) )
                            offMessage = tr( "Not scrobbling - excluded directory" );
                        else if ( m_track.isVideo() )
                            offMessage = tr( "Not scrobbling - not a music video" );
                        else if ( !unicorn::UserSettings().value( "podcasts", true ).toBool() && m_track.isPodcast() )
                            offMessage = tr( "Not scrobbling - podcasts disabled" );
                        else if ( m_track.artist().isNull() )
                            offMessage = tr( "Not scrobbling - missing artist" );
                    }

                    if(offMessage != NULL)
                    {
                        p.setPen( QColor( 0x333333 ) );
                        QTextOption textOption;
                        textOption.setAlignment( Qt::AlignVCenter | Qt::AlignRight );
                        p.drawText( rect().adjusted( 0, 0, -6, 0 ), offMessage, textOption );
                    }
                }
            }
            else
            {
                QTextOption textOption;
                textOption.setAlignment( Qt::AlignVCenter | Qt::AlignRight );
                p.drawText( rect().adjusted( 0, 0, -6, 0 ), tr( "Not scrobbling - track too short" ), textOption );
            }
        }
        else
        {
            QTextOption textOption;
            textOption.setAlignment( Qt::AlignVCenter | Qt::AlignRight );
            p.drawText( rect().adjusted( 0, 0, -6, 0 ),
                    tr("Enable scrobbling in %1's preferences!").arg( m_track.extra( "playerName" ) ),
                    textOption );
        }
    }
}
        
