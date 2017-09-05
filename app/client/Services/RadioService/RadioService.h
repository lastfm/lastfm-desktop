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
#ifndef RADIO_SERVICE_H
#define RADIO_SERVICE_H

#include <lastfm/Track.h>
#include <lastfm/RadioStation.h>
#include <lastfm/RadioTuner.h>
#include <lastfm/ws.h>
#include <phonon/phononnamespace.h>
#include <phonon/audiooutput.h>
#include <phonon/mediasource.h>
#include <QList>
#include <QPointer>
#include <QVariant>
#include "lib/listener/State.h"

namespace Phonon
{
	class MediaObject;
	class AudioOutput;
	class MediaSource;
    class Path;
}

namespace unicorn { class Session; }

/** @author <max@last.fm>
 */
class RadioService : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)

public:
    RadioService();

    RadioStation station() const { return m_station; }
    Track currentTrack() const {return m_track;}

    State state() const { return m_state; }

    Phonon::AudioOutput* audioOutput() const { return m_audioOutput; }
    Phonon::MediaObject* mediaObject() const { return m_mediaObject; }

    static RadioService& instance(){ static RadioService r; return r; }

public slots:
    void play( const RadioStation& station );
    void playNext( const RadioStation& station );
    void skip();
    void stop();
    void volumeUp();
    void volumeDown();
    void mute();
    void pause();
    void resume();
    void setSupportsDisco( bool supportsDisco );
    void setMaxUsageCount(int count);

signals:
    /** emitted up to twice, as first time may not have a title for the station
      * but the second time will */
    void tuningIn( const RadioStation& );
    void trackSpooled( const Track& );
    void buffering( int );
    void paused();
    void resumed();
    void stopped();
    void supportsDisco( bool supportsDisco );
	
    /** the error is one of lastfm::ws::Error,
      * if UnknownError, then data is a fatal error from Phonon */
    void error( int, const QVariant& data = QVariant() );
    void tick( qint64 );
    void message( const QString& message );

private slots:
    void onSessionChanged( const unicorn::Session& session );

    void enqueue();
    void onPhononStateChanged( Phonon::State, Phonon::State );
    void onPhononCurrentSourceChanged( const Phonon::MediaSource &);
    void onTunerError( lastfm::ws::Error, const QString& message );
    void phononEnqueue();
    void onBuffering( int );
    void onFinished();

    void onMutedChanged(bool);
    void onOutputDeviceChanged(const Phonon::AudioOutputDevice&);
    void onVolumeChanged(qreal);

    void onLastFmUrl( const QUrl& url );

	/** we get a "proper" station name from the tune webservice */
    void setStationName( const QString& );

    void volumeChange( double step );
private:
    /** resets internals to what Stopped means, used by changeState() */
    void clear();
    bool initRadio();
    void deInitRadio();

    void restoreVolume();
    
    /** emits signals if appropriate */
    void changeState( State );
	
    QPointer<lastfm::RadioTuner> m_tuner;
    Phonon::AudioOutput* m_audioOutput;
    Phonon::MediaObject* m_mediaObject;
    Phonon::Path m_path;
    State m_state;
    Track m_track;
    RadioStation m_station;
    bool m_bErrorRecover;
    int m_maxUsageCount;
    QString m_currentUser;
};


#include <QDebug>
inline QDebug operator<<( QDebug d, Phonon::State s )
{
	switch (s)
	{
	    #define CASE(x) case x: return d << #x
		CASE(Phonon::LoadingState);
		CASE(Phonon::StoppedState);
		CASE(Phonon::PlayingState);
		CASE(Phonon::BufferingState);
		CASE(Phonon::PausedState);
		CASE(Phonon::ErrorState);
        #undef CASE
	}
    return d;
}


#endif //RADIO_SERVICE_H_
