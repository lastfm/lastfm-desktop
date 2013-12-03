/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Jono Cole

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

#import "FmLastPrefPaneQtView.h"
#include "FmLastPrefPanePrefWidget.h"

@implementation FmLastPrefPaneQtView

- (id) initWithFrame:(NSRect)frameRect {
    [super initWithFrame: frameRect];
    
    pw = new FmLastPrefPanePrefWidget;
    NSView* pwView = pw->view();
    [self addSubview: pwView positioned:NSWindowAbove relativeTo:nil];
//    [pwView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
//    [pwView setAutoresizesSubviews:YES];

    [self drawRect: [self frame]];
    return self;
}

- (void) drawRect:(NSRect)rect {
    NSView* pwView = pw->view();
    [pwView setFrameOrigin: NSMakePoint( 0, [self frame].size.height - [pwView frame].size.height)];
    [pwView setFrameSize: [self frame].size];    
    
    [super drawRect: rect];
}

- (FmLastPrefPanePrefWidget*) prefWidget {
    return pw;
}

@end
