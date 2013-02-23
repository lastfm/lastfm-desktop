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

#include <QThread>
#include <QTimer>
#include <QDesktopServices>
#include <QCoreApplication>

#include <phonon/mediaobject.h>
#include <phonon/backendcapabilities.h>

#include <lastfm/RadioTuner.h>

#include "lib/unicorn/UnicornSettings.h"

#include "../../Application.h"
#include "RadioService.h"

RadioService::RadioService()
     : m_audioOutput( 0 ),
       m_mediaObject( 0 ),
       m_state( Stopped ),
       m_bErrorRecover( false ),
       m_maxUsageCount( 180 )
{
    initRadio();

    QDesktopServices::setUrlHandler( "lastfm", this, "onLastFmUrl" );

    onSessionChanged( aApp->currentSession() );

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );
}

void
RadioService::onSessionChanged( const unicorn::Session& session )
{
    if ( session.user().name() != m_currentUser )
    {
        m_currentUser = session.user().name();

        // if they change user, make sure we stop the radio
        if ( m_mediaObject && m_mediaObject->state() != Phonon::StoppedState )
            stop();
    }
}

void
RadioService::onLastFmUrl( const QUrl& url )
{
    RadioStation rs( url.toString() );
    play( rs );
}

// fixme:
// todo:
// note:
// if the station is the same as current station (ie: the user hit stop then start)
// then we *don't* retune.  norman is quite emphatic about this.  :)
void
RadioService::play( const RadioStation& station )
{  
    if ( !aApp->currentSession().youRadio() )
    {
        // they are not a subscriber so don't let them start the radio
        emit error( lastfm::ws::SubscribersOnly, tr( "You need to be a subscriber to listen to radio" ) );
        return;
    }

    if( m_state == Paused
            && (station.url() == "" || station.url() == m_station.url() ) )
    {
        m_mediaObject->play();
        return;
    }

    if( m_state != Stopped
            && ( station.url() == "" || station.url() == m_station.url() ) )
    {
        // do nothing
        return;
    }

    if (m_state != Stopped)
    {
        //FIXME filthy! get us to a clean slate
        State oldstate = m_state;
        m_state = Stopped;    //prevents stateChanged() doing anything
        stop();
        clear();
        m_state = oldstate;
    }

    delete m_tuner;

    if (0 == m_audioOutput)  {
        if (!initRadio()) {
            changeState( Stopped );
            return;
        }
    }

    m_station = station;

    if ( m_station.url() == "" )
    {
        unicorn::UserSettings us;
        QString stationUrl = us.value( "lastStationUrl", "" ).toString();
        QString stationTitle = us.value( "lastStationTitle", tr( "A Radio Station" ) ).toString();

        m_station.setUrl( stationUrl );
        m_station.setTitle( stationTitle );
    }

    // Make sure the radio station has the radio options from the settings
    bool ok;
    m_station.setRep( unicorn::AppSettings().value( "rep", 0.5 ).toDouble( &ok ) );
    m_station.setMainstr( unicorn::AppSettings().value( "mainstr", 0.5 ).toDouble( &ok ) );
    m_station.setDisco( unicorn::AppSettings().value( "disco", false ).toBool() );

    m_tuner = new lastfm::RadioTuner( m_station );

    connect( m_tuner, SIGNAL(title( QString )), SLOT(setStationName( QString )) );
    connect( m_tuner, SIGNAL(supportsDisco( bool )), SLOT(setSupportsDisco( bool )) );
    connect( m_tuner, SIGNAL(trackAvailable()), SLOT(enqueue()) );
    connect( m_tuner, SIGNAL(error( lastfm::ws::Error, QString )), SLOT(onTunerError( lastfm::ws::Error, QString )) );

    changeState( TuningIn );
}

// play this radio station after the current track has finished
void
RadioService::playNext( const RadioStation& station )
{
    if (m_state == Playing)
    {
        m_station = station;

        if ( m_station.url() == "" )
        {
            unicorn::UserSettings us;
            QString stationUrl = us.value( "lastStationUrl", "" ).toString();
            QString stationTitle = us.value( "lastStationTitle", tr( "A Radio Station" ) ).toString();

            m_station.setUrl( stationUrl );
            m_station.setTitle( stationTitle );
        }

        // Make sure the radio station has the radio options from the settings
        bool ok;
        m_station.setRep( unicorn::AppSettings().value( "rep", 0.5 ).toDouble( &ok ) );
        m_station.setMainstr( unicorn::AppSettings().value( "mainstr", 0.5 ).toDouble( &ok ) );
        m_station.setDisco( unicorn::AppSettings().value( "disco", false ).toBool() );

        m_tuner->retune( m_station );
    }
}


void
RadioService::enqueue()
{  
    qDebug() << "enqueue";
    if (m_state == Stopped) {
        // this should be impossible. If we are stopped, then the GUI looks
        // stopped too, and receiving tracks to play will result in a playing
        // radio and a stopped GUI. NICE.
        Q_ASSERT( 0 );
        return;
    }
	
    phononEnqueue();
}

void
RadioService::skip()
{
    if (!m_mediaObject)
        return;
    
    // attempt to refill the phonon queue if it's empty
    if (m_mediaObject->queue().isEmpty())
        phononEnqueue();
    
    QList<Phonon::MediaSource> q = m_mediaObject->queue();

    if ( q.size() )
    {
        m_mediaObject->setCurrentSource( q[0] );

        //if the error returns a 403 permission denied error, the mediaObject is uninitialised
        if( m_mediaObject )
            m_mediaObject->play();
        else {
            initRadio();
            play( RadioStation());
        }
    }
    else if (m_state != Stopped)
    {
        qDebug() << "queue empty";
        // we are still waiting for a playlist to come back from the tuner
        m_mediaObject->blockSignals( true );    //don't tell outside world that we stopped
        m_mediaObject->stop();
        m_mediaObject->setCurrentSource( QUrl() );
        m_mediaObject->blockSignals( false );
        changeState( TuningIn );
    }
}


void
RadioService::onTunerError( lastfm::ws::Error e, const QString& message )
{
    // otherwise leave things be, we'll stop when we run out of content
    if (m_state == TuningIn)
        stop();

    emit error( e, message );
}


void
RadioService::stop()
{
    delete m_tuner;
    
    m_mediaObject->blockSignals( true ); //prevent the error state due to setting current source to null
    m_mediaObject->stop();
    m_mediaObject->clearQueue();
    m_mediaObject->setCurrentSource( QUrl() );
    m_mediaObject->blockSignals( false );

    clear();
    
    changeState( Stopped );
}


void
RadioService::pause()
{
    Q_ASSERT( m_mediaObject );

    if ( m_mediaObject )
    {
        m_mediaObject->pause();
        changeState( Paused );
    }
}

void
RadioService::resume()
{
    Q_ASSERT( m_mediaObject );

    if ( m_mediaObject )
    {
        m_mediaObject->play();
        changeState( Playing );
    }
}


void
RadioService::clear()
{
    m_track = Track();
    m_station = RadioStation();
    delete m_tuner;
}

void
RadioService::volumeUp()
{

}

void
RadioService::volumeDown()
{
}

void
RadioService::mute()
{
    m_audioOutput->setMuted( !m_audioOutput->isMuted() );
}

void
RadioService::onPhononStateChanged( Phonon::State newstate, Phonon::State oldstate )
{
    qDebug() << oldstate << " -> " << newstate;
    if (m_mediaObject == 0) {
        qDebug() << "m_mediaObject is null!";
        return;
    }

    switch (newstate)
    {
        case Phonon::ErrorState:
            qWarning() << m_mediaObject->errorType() << m_mediaObject->errorString();
            emit error( lastfm::ws::UnknownError, QVariant( m_mediaObject->errorString() ));
            deInitRadio();
            changeState( Stopped );
            break;
			
        case Phonon::PausedState:
            // if the play queue runs out we get this for some reason
            // this means we are fetching new tracks still, we should show a
            // tuning in state;
            if (m_mediaObject->queue().size() == 0)
            {
                qDebug() << "queue empty, going to TuningIn";
                changeState( Paused );
            }
            break;
			
        case Phonon::StoppedState:
            if (m_bErrorRecover)
            {
                m_bErrorRecover = false;
                skip();
            }
            break;
			
        case Phonon::BufferingState:
            changeState( Buffering );
            restoreVolume();
            break;

        case Phonon::PlayingState:
            changeState( Playing );
            break;

        case Phonon::LoadingState:
            break;
    }
}

void
RadioService::restoreVolume()
{
    // restore the last volume
    if ( unicorn::AppSettings().contains("Volume") )
    {
        bool ok;
        double volume = unicorn::AppSettings().value( "Volume", 1 ).toReal(&ok);
        if (ok)
            m_audioOutput->setVolume(volume);
        else
            m_audioOutput->setVolume( 1 );
    }
    else
        m_audioOutput->setVolume( 1 );

    m_audioOutput->setMuted(unicorn::AppSettings().value("Muted", false).toBool());
}

void
RadioService::phononEnqueue()
{
    qDebug() << "phononEnqueue";
    qDebug() << "queue size: " << m_mediaObject->queue().size();

    if (m_mediaObject->queue().size() || !m_tuner)
        return;

    // keep only one track in the phononQueue
    // Loop until we get a null url or a valid url.
    forever
    {
        // consume next track from the track source. a null track 
        // response means wait until the trackAvailable signal
        Track t = m_tuner->takeNextTrack();

        if (t.isNull())
        {
            m_track = t;
            changeState( TuningIn );
            break;
        }

        // Invalid urls won't trigger the correct phonon
        // state changes, so we must filter them.
        if (!t.url().isValid())
            continue;
        
        m_track = t;
        Phonon::MediaSource ms( t.url() );

        qDebug() << "enqueuing " << t;
        try
        {
            m_mediaObject->enqueue( ms );
            m_mediaObject->play();
        }
        catch (...)
        {
            continue;
        }

        break;
    }
}


// onPhononCurrentSourceChanged happens always (even if the source is
// unplayable), so we use it to update our now playing track.
void
RadioService::onPhononCurrentSourceChanged( const Phonon::MediaSource& )
{
    if (m_mediaObject == 0) {
        qDebug() << "m_mediaObject is null!";
        return;
    }

    if ( !m_track.isNull() )
    {
        MutableTrack( m_track ).stamp();

        if (m_mediaObject->state() != Phonon::PlayingState)
            changeState( Buffering );

        emit trackSpooled( m_track );
    }
}


void
RadioService::changeState( State const newstate )
{
    State const oldstate = m_state;

    if (oldstate == newstate) 
        return;

    qDebug().nospace() << newstate << " (was " << oldstate << ')';
     
    m_state = newstate; // always assign state properties before you tell other
                        // objects about it
    
    switch (newstate)
	{
    case TuningIn:
        qDebug() << "Tuning to:" << m_station;
        emit tuningIn( m_station );
        break;

    default:
    case Buffering:
        break;
    case Playing:
        emit resumed();
        break;

    case Stopped:
        emit stopped();
        break;

    case Paused:
        emit paused();
        break;
	}
}

void
RadioService::setStationName( const QString& s )
{
    m_station.setTitle( s );

    unicorn::UserSettings us;
    us.setValue( "lastStationUrl", m_station.url() );
    us.setValue( "lastStationTitle", m_station.title() );
}

void
RadioService::setSupportsDisco( bool supportsDiscovery )
{
    emit supportsDisco( supportsDiscovery );
}

void
RadioService::setMaxUsageCount(int count)
{
    m_maxUsageCount = count;
}

void
RadioService::onBuffering( int percent_filled )
{
    Q_UNUSED(percent_filled);
    emit buffering( percent_filled );
}

void
RadioService::onMutedChanged(bool muted)
{
    unicorn::AppSettings().setValue("Muted", muted);
}

void
RadioService::onOutputDeviceChanged(const Phonon::AudioOutputDevice& newDevice)
{
    qDebug() << "name: " << newDevice.name() << " description: " << newDevice.description();
}

void
RadioService::onVolumeChanged(qreal vol)
{
    unicorn::AppSettings().setValue( "Volume", vol );
}

void
RadioService::onFinished()
{
    // A track has finished and there is nothing else in the queue
    // try to go to the next track
    skip();
}


// returns true on successful initialisation
bool 
RadioService::initRadio()
{
    qDebug() << "initRadio";
    Phonon::AudioOutput* audioOutput = new Phonon::AudioOutput( Phonon::MusicCategory );

    qDebug() << audioOutput->name();
    qDebug() << audioOutput->outputDevice().description();
    qDebug() << audioOutput->outputDevice().name();
    qDebug() << (audioOutput->isMuted() ? "muted,": "not-muted,") <<
                (audioOutput->isValid() ? "valid,": "not-valid,") <<
                audioOutput->volumeDecibel() << "db " <<
                audioOutput->volume();
    foreach (QByteArray a, audioOutput->outputDevice().propertyNames()) {
        qDebug() << a << ":" << audioOutput->outputDevice().property(a);
    }

    QString audioOutputDeviceName = "";//TODO moose::Settings().audioOutputDeviceName();
    if (audioOutputDeviceName.size())
    {
        foreach (Phonon::AudioOutputDevice d, Phonon::BackendCapabilities::availableAudioOutputDevices())
            if (d.name() == audioOutputDeviceName) {
                audioOutput->setOutputDevice( d );
                break;
            }
    }

    Phonon::MediaObject* mediaObject = new Phonon::MediaObject;
    m_path = Phonon::createPath( mediaObject, audioOutput );
    if (!m_path.isValid()) {
        qDebug() << "Phonon::createPath failed";
// can't delete the mediaObject after a failed Phonon::createPath without a crash in phonon...  (Qt 4.4.3)
// so, we leak it:
//        mediaObject->deleteLater();
        audioOutput->deleteLater();
        return false;
    } 

    mediaObject->setTickInterval( 500 );
    connect( mediaObject, SIGNAL(stateChanged( Phonon::State, Phonon::State )), SLOT(onPhononStateChanged( Phonon::State, Phonon::State )) );
    connect( mediaObject, SIGNAL(bufferStatus(int)), SLOT(onBuffering(int)));
    connect( mediaObject, SIGNAL(currentSourceChanged( Phonon::MediaSource )), SLOT(onPhononCurrentSourceChanged( Phonon::MediaSource )) );
    connect( mediaObject, SIGNAL(aboutToFinish()), SLOT(phononEnqueue()) ); // this fires when the whole queue is about to finish
    connect( mediaObject, SIGNAL(finished()), SLOT(onFinished()));
    connect( mediaObject, SIGNAL(tick(qint64)), SIGNAL(tick(qint64)));
    connect( audioOutput, SIGNAL(mutedChanged(bool)), SLOT(onMutedChanged(bool)));
    connect( audioOutput, SIGNAL(outputDeviceChanged(Phonon::AudioOutputDevice)), SLOT(onOutputDeviceChanged(Phonon::AudioOutputDevice)));
    connect( audioOutput, SIGNAL(volumeChanged(qreal)), SLOT(onVolumeChanged(qreal)));

    m_audioOutput = audioOutput;
    m_mediaObject = mediaObject;

    restoreVolume();

    return true;
}

void
RadioService::deInitRadio()
{
    qDebug() << "deInitRadio";
    // try to deleteLater and phonon crashes. poo.
    // leak em...  :(
    
    m_audioOutput->deleteLater();
    m_audioOutput = 0;
    m_mediaObject->deleteLater();
    m_mediaObject = 0;
}
