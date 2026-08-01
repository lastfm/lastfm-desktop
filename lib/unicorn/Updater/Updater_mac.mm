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

#include <QStringList>

#include <../UnicornApplication.h>
#include <../UnicornSettings.h>

#include "Updater.h"

#include <AppKit/NSNibDeclarations.h>
#include <Foundation/Foundation.h>
#include <Foundation/NSURL.h>

#ifdef HAVE_SPARKLE

#include <Sparkle/Sparkle.h>
#include <Sparkle/SUUpdater.h>

@interface UpdaterDelegate : NSObject {
}
@end

@implementation UpdaterDelegate
@end

UpdaterDelegate* g_Delegate;

unicorn::Updater::Updater(QWidget *parent) :
    QObject(parent)
{
    SUUpdater* updater = [SUUpdater sharedUpdater];
    g_Delegate = [UpdaterDelegate alloc];
    [updater setDelegate:g_Delegate];

    if ( qApp->arguments().contains( "--update" ) )
    {
        int urlIndex = qApp->arguments().indexOf( "--update" ) + 1;

        if ( qApp->arguments().count() > urlIndex && qApp->arguments()[urlIndex].startsWith( "https://" ) )
            [updater setFeedURL:[NSURL URLWithString: [NSString stringWithCharacters:(const unichar *)qApp->arguments()[urlIndex].unicode() length:(NSUInteger)qApp->arguments()[urlIndex].length() ]]];
        else
            setBetaUpdates( unicorn::Settings().betaUpdates() );
    }
    else
        setBetaUpdates( unicorn::Settings().betaUpdates() );

}

void
unicorn::Updater::setBetaUpdates( bool betaUpdates )
{
    if ( betaUpdates )
        [[SUUpdater sharedUpdater] setFeedURL:[NSURL URLWithString:UPDATE_URL_MAC_BETA]];
    else
        [[SUUpdater sharedUpdater] setFeedURL:[NSURL URLWithString:UPDATE_URL_MAC]];

}

void
unicorn::Updater::checkForUpdates()
{
    [[SUUpdater sharedUpdater] checkForUpdates:0];
}

#else // !HAVE_SPARKLE - Sparkle.framework not found in /Library/Frameworks or
      // ~/Library/Frameworks at qmake time (admin/qmake/3rdparty.pro.inc), so
      // this build cannot update itself. Fall back to the download page so the
      // "Check for Updates" menu item stays honest.

#warning "Sparkle.framework not found: auto-update is compiled out of this build"

#include <QDebug>
#include <QUrl>
#include "../DesktopServices.h"

unicorn::Updater::Updater(QWidget *parent) :
    QObject(parent)
{
    qDebug() << "Built without Sparkle: auto-update is unavailable";
}

void
unicorn::Updater::setBetaUpdates( bool )
{
}

void
unicorn::Updater::checkForUpdates()
{
    qDebug() << "Built without Sparkle: sending the user to the download page instead";
    unicorn::DesktopServices::openUrl( QUrl( "https://www.last.fm/about/trackmymusic" ) );
}

#endif

unicorn::Updater::~Updater()
{
    // do nothing - included here for header compatibility with windows version
}
