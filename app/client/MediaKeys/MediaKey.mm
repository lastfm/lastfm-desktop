/*
   Copyright 2011 Last.fm Ltd.
      - Primarily authored by Michael Coffey

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

#include <QAction>

#include "lib/unicorn/UnicornSettings.h"

#include "../Services/ScrobbleService/ScrobbleService.h"
#include "../Application.h"
#include "MediaKey.h"

#include "lib/3rdparty/SPMediaKeyTap/SPMediaKeyTap.h"

#import <Cocoa/Cocoa.h>


@interface MediaKeyTapDelegate : NSObject {
    SPMediaKeyTap* keyTap;
}
@end

@implementation MediaKeyTapDelegate

+(void)initialize
{
    if([self class] != [MediaKeyTapDelegate class])
        return;

    // Register defaults for the whitelist of apps that want to use media keys
    [[NSUserDefaults standardUserDefaults] registerDefaults:[NSDictionary dictionaryWithObjectsAndKeys:
        [SPMediaKeyTap defaultMediaKeyUserBundleIdentifiers], kMediaKeyUsingBundleIdentifiersDefaultsKey,
    nil]];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification lastTrackRadio:(bool)lastTrackRadio
{
    qDebug() << "Did finish launching!";

    keyTap = [[SPMediaKeyTap alloc] initWithDelegate:self];

    bool actualEnabled = unicorn::Settings().value( "mediaKeys", true ).toBool() && aApp->currentSession().youRadio() && lastTrackRadio;

    if ( [SPMediaKeyTap usesGlobalMediaKeyTap] && actualEnabled )
        [keyTap startWatchingMediaKeys];
    else
        NSLog(@"Media key monitoring diabled");
}

-(void)mediaKeyTap:(SPMediaKeyTap*)keyTap receivedMediaKeyEvent:(NSEvent*)event
{
    NSAssert([event type] == NSSystemDefined && [event subtype] == SPSystemDefinedEventMediaKeys, @"Unexpected NSEvent in mediaKeyTap:receivedMediaKeyEvent:");
    // here be dragons...
    int keyCode = (([event data1] & 0xFFFF0000) >> 16);
    int keyFlags = ([event data1] & 0x0000FFFF);
    BOOL keyIsPressed = (((keyFlags & 0xFF00) >> 8)) == 0xA;
    int keyRepeat = (keyFlags & 0x1);

    if ( keyIsPressed && !keyRepeat )
    {
        QString debugString = QString( "key repeated: %1 " ).arg( keyRepeat );

        switch (keyCode)
        {
            case NX_KEYTYPE_PLAY:
                debugString.append( "Play/pause pressed" );
                aApp->playAction()->trigger();
                break;

            case NX_KEYTYPE_FAST:
            case NX_KEYTYPE_NEXT:
                debugString.append( "Ffwd pressed" );
                aApp->skipAction()->trigger();
                break;

            case NX_KEYTYPE_REWIND:
                debugString.append( "Rewind pressed" );
                break;
            default:
                debugString.append( QString( "Key %1 pressed" ).arg( keyCode ) );
                break;
            // More cases defined in hidsystem/ev_keymap.h
        }

        qDebug() << debugString;
    }
}

- (void)setEnabled:(bool)enabled
{
    if ( [SPMediaKeyTap usesGlobalMediaKeyTap] && enabled )
    {
        qDebug() << "Start watching media keys";
        [keyTap startWatchingMediaKeys];
    }
    else
    {
        qDebug() << "Stop watching media keys";
        [keyTap stopWatchingMediaKeys];
    }
}

@end

MediaKeyTapDelegate* g_tapDelegate;

MediaKey::MediaKey( QObject* parent )
    :QObject( parent ), m_lastTrackRadio( !aApp->arguments().contains( "--tray" ) )
{
    g_tapDelegate = [MediaKeyTapDelegate alloc];

    connect( aApp->delegate(), SIGNAL(initialize()), SLOT(initialize()) );
    connect( aApp->delegate(), SIGNAL(applicationDidFinishLaunching(void*)), SLOT(applicationDidFinishLaunching(void*)) );

    connect( aApp, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onSessionChanged(unicorn::Session)) );

    connect( &ScrobbleService::instance(), SIGNAL(trackStarted(lastfm::Track,lastfm::Track)), SLOT(onTrackStarted(lastfm::Track,lastfm::Track)) );
}

void MediaKey::onSessionChanged( const unicorn::Session& session )
{
    bool actualEnabled = unicorn::Settings().value( "mediaKeys", true ).toBool() && session.youRadio() && m_lastTrackRadio;
    [g_tapDelegate setEnabled:actualEnabled];
}

bool
MediaKey::macEventFilter( EventHandlerCallRef, EventRef event )
{
    NSEvent* theEvent = reinterpret_cast<NSEvent *>(event);

    // If event tap is not installed, handle events that reach the app instead
    BOOL shouldHandleMediaKeyEventLocally = ![SPMediaKeyTap usesGlobalMediaKeyTap];

    if(shouldHandleMediaKeyEventLocally
            && [theEvent type] == NSSystemDefined
            && [theEvent subtype] == SPSystemDefinedEventMediaKeys)
    {
        [(id)g_tapDelegate mediaKeyTap:nil receivedMediaKeyEvent:theEvent];
    }

    return false;
}

void
MediaKey::initialize()
{
    [g_tapDelegate initialize];
}

void
MediaKey::applicationDidFinishLaunching( void* aNotification )
{
    [g_tapDelegate applicationDidFinishLaunching:static_cast<NSNotification*>(aNotification) lastTrackRadio:m_lastTrackRadio];
}

void
MediaKey::setEnabled( bool enabled )
{
    bool actualEnabled = enabled && aApp->currentSession().youRadio() && m_lastTrackRadio;
    [g_tapDelegate setEnabled:actualEnabled];
}


void
MediaKey::onTrackStarted( const lastfm::Track& newTrack, const lastfm::Track& /*oldTrack*/ )
{
    m_lastTrackRadio = newTrack.source() == Track::LastFmRadio;
    bool actualEnabled = unicorn::Settings().value( "mediaKeys", true ).toBool() && aApp->currentSession().youRadio() && m_lastTrackRadio;
    [g_tapDelegate setEnabled:actualEnabled];
}
