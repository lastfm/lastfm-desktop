/*
   Copyright 2012 Last.fm Ltd.
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

#include "UnicornSettings.h"
#include "UnicornApplicationDelegate.h"

#include <QWidget>
#include <QApplication>
#include <QDebug>

#import <Cocoa/Cocoa.h>
#import <AppKit/NSView.h>
#import <Carbon/Carbon.h>

// Declare this here ourselves so we can compile on OSX 10.6
// and ProcessTransformToUIElementApplication just won't work
enum {
  ProcessTransformToForegroundApplication = 1,
  ProcessTransformToBackgroundApplication = 2, /* functional in Mac OS X Barolo and later */
  ProcessTransformToUIElementApplication = 4 /* functional in Mac OS X Barolo and later */
};

@interface LFMAppDelegate : NSObject <NSApplicationDelegate> {
    unicorn::UnicornApplicationDelegate* m_observer;
    BOOL m_show;
}
- (LFMAppDelegate*) init:(unicorn::UnicornApplicationDelegate*)observer;
@end

@implementation LFMAppDelegate

- (LFMAppDelegate*) init:(unicorn::UnicornApplicationDelegate*)observer
{
    if ( (self = [super init]) )
    {
        self->m_observer = observer;
    }

    return self;
}

-(void)initialize
{
    m_observer->forceInitialize();
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
    qDebug() << "applicationWillFinishLaunching";

    if ( unicorn::Settings().showDock() )
    {
        ProcessSerialNumber psn = { 0, kCurrentProcess };
        TransformProcessType(&psn, ProcessTransformToForegroundApplication);
        SetSystemUIMode(kUIModeNormal, 0);
        [[NSWorkspace sharedWorkspace] launchAppWithBundleIdentifier:@"com.apple.dock" options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifier:nil];
        [[NSApplication sharedApplication] activateIgnoringOtherApps:TRUE];
    }
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    m_observer->forceApplicationDidFinishLaunching( aNotification );
}


- (BOOL)application:(NSApplication*)sender delegateHandlesKey:(NSString*)key
{
    Q_UNUSED(sender);
    return [[NSSet setWithObjects: @"trackTitle", @"artist", @"album", @"duration", @"artwork", @"loved", nil] containsObject:key];
}

- (NSString*)trackTitle
{
    if ( m_observer->m_observer )
    {
        QString string = m_observer->m_observer->trackTitle();

        if ( !string.isEmpty() )
            return [NSString stringWithCharacters:(const unichar *)string.unicode() length:(NSUInteger)string.length() ];
    }

    return nil;
}

- (NSString*)artist
{
    if ( m_observer->m_observer )
    {
        QString string = m_observer->m_observer->artist();

        if ( !string.isEmpty() )
            return [NSString stringWithCharacters:(const unichar *)string.unicode() length:(NSUInteger)string.length() ];
    }

    return nil;
}

- (NSString*)album
{
    if ( m_observer->m_observer )
    {
        QString string = m_observer->m_observer->album();

        if ( !string.isEmpty() )
            return [NSString stringWithCharacters:(const unichar *)string.unicode() length:(NSUInteger)string.length() ];
    }

    return nil;
}

- (NSNumber*)duration
{
    return [NSNumber numberWithInt:m_observer->m_observer ? m_observer->m_observer->duration() : 0];
}

- (NSData*)artwork
{
    if ( m_observer->m_observer )
    {
        QPixmap pixmap = m_observer->m_observer->artwork();

        if ( !pixmap.isNull() )
        {
            CGImageRef cgImage = pixmap.toMacCGImageRef();
            NSImage* nsImage = [[NSImage alloc] initWithCGImage:(CGImageRef)cgImage size:(NSSize)NSZeroSize];
            NSData* data = [nsImage TIFFRepresentation];
            return data;
        }
        else
        {
            NSImage* img = [NSImage imageNamed: NSImageNameApplicationIcon];
            NSData* data = [img TIFFRepresentation];
            return data;
        }
    }

    return nil;
}

- (BOOL)loved
{
    return m_observer->m_observer && m_observer->m_observer->loved() ? YES : NO;
}

@end

LFMAppDelegate* g_appDelegate;

unicorn::UnicornApplicationDelegate::UnicornApplicationDelegate(QObject *parent) :
    QObject(parent), m_observer( 0 )
{
    g_appDelegate = [[LFMAppDelegate alloc] init:this];

    [[NSApplication sharedApplication] setDelegate: g_appDelegate];
}

void
unicorn::UnicornApplicationDelegate::showDockIcon( bool show )
{
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, show?ProcessTransformToForegroundApplication:ProcessTransformToUIElementApplication);
    SetSystemUIMode(kUIModeNormal, 0);
    [[NSWorkspace sharedWorkspace] launchAppWithBundleIdentifier:@"com.apple.dock" options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifier:nil];
    [[NSApplication sharedApplication] activateIgnoringOtherApps:TRUE];
}

void
unicorn::UnicornApplicationDelegate::setCommandObserver( UnicornApplicationDelegateCommandObserver* observer )
{
    m_observer = observer;
}

void
unicorn::UnicornApplicationDelegate::forceInitialize()
{
    emit initialize();
}

void
unicorn::UnicornApplicationDelegate::forceApplicationDidFinishLaunching( void* aNotification )
{
    emit applicationDidFinishLaunching( aNotification );
}

