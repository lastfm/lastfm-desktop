/*
   Copyright 2010-2012 Last.fm Ltd.
      - Primarily authored by Jono Cole and Michael Coffey

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

#include "UnicornApplication.h"
#import <Foundation/NSAppleEventDescriptor.h>
#import <Foundation/NSAppleEventManager.h>
#import <Foundation/NSObject.h>

void 
unicorn::Application::setOpenApplicationEventHandler()
{
    NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];
    [appleEventManager setEventHandler:NULL andSelector:NULL forEventClass:kCoreEventClass andEventID:kAEReopenApplication];
}

void
unicorn::Application::setGetURLEventHandler()
{
    NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];
    [appleEventManager setEventHandler:NULL andSelector:NULL forEventClass:kInternetEventClass andEventID:kAEGetURL];
}

void
unicorn::Application::macTranslate( const QString& lang )
{
    NSString* langString = [NSString stringWithCharacters:(const unichar *)lang.unicode() length:(NSUInteger)lang.length() ];

    [[NSUserDefaults standardUserDefaults] setObject:[NSArray arrayWithObject:langString] forKey:@"AppleLanguages"];
}

