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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#ifdef LASTFM_USER_NOTIFICATIONS
#include <Foundation/NSUserNotification.h>
#endif

#include <QDebug>
#include <QPixmap>

#include <lastfm/Track.h>

#include "Notify.h"


#ifdef LASTFM_USER_NOTIFICATIONS
@interface MacClickDelegate : NSObject <NSUserNotificationCenterDelegate> {
    unicorn::Notify* m_observer;
}
    - (MacClickDelegate*) initialise:(unicorn::Notify*)observer;
    - (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification;
@end

@implementation MacClickDelegate
- (MacClickDelegate*) initialise:(unicorn::Notify*)observer
{
    if ( (self = [super init]) )
    {
        self->m_observer = observer;
    }

    return self;
}

- (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    Q_UNUSED(center)
    Q_UNUSED(notification)
    self->m_observer->notificationWasClicked();
}
@end
#endif

unicorn::Notify::Notify(QObject *parent) :
    QObject(parent)
{
#ifdef LASTFM_USER_NOTIFICATIONS
    if ( [NSUserNotificationCenter class] )
    {
        MacClickDelegate* macDelegate = [[MacClickDelegate alloc] initialise: this];
        [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:macDelegate];
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
    }
#endif
}

unicorn::Notify::~Notify()
{
#ifdef LASTFM_USER_NOTIFICATIONS
    if ( [NSUserNotificationCenter class] )
    {
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
    }
#endif
}

void
unicorn::Notify::newTrack( const lastfm::Track& track )
{
    delete m_trackImageFetcher;
    m_trackImageFetcher = new TrackImageFetcher( track, Track::LargeImage );
    connect( m_trackImageFetcher, SIGNAL(finished(QPixmap)), SLOT(onFinished(QPixmap)) );

#ifdef LASTFM_USER_NOTIFICATIONS
    if ( [NSUserNotificationCenter class] )
    {
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
        onFinished( QPixmap() );
    }
    else
#endif
    {
        // No notification backend: NSUserNotificationCenter is gone from the
        // OS. Don't fetch artwork nothing will display.
        qDebug() << "Notify: no notification backend available, dropping notification for" << track.toString();
    }
}

void
unicorn::Notify::paused()
{
#ifdef LASTFM_USER_NOTIFICATIONS
    if ( [NSUserNotificationCenter class] )
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
#endif
}

void
unicorn::Notify::resumed()
{
#ifdef LASTFM_USER_NOTIFICATIONS
    if ( [NSUserNotificationCenter class] )
    {
        if ( m_trackImageFetcher )
        {
            [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
            onFinished( QPixmap() );
        }
    }
#endif
}

void
unicorn::Notify::stopped()
{
#ifdef LASTFM_USER_NOTIFICATIONS
    if ( [NSUserNotificationCenter class] )
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
#endif

    // m_trackImageFetcher is a QPointer so this nulls it too
    delete m_trackImageFetcher;
}

void
unicorn::Notify::onFinished( const QPixmap& pixmap )
{
    // Notification Center has no use for the artwork pixmap; the parameter
    // stays because TrackImageFetcher's finished(QPixmap) signal delivers it
    Q_UNUSED( pixmap )

    Track track = m_trackImageFetcher->track();

    QString title = track.title();
    QString description = tr("%1\n%2").arg( track.artist(), track.album() );

    if ( track.album().isNull() )
        description = track.artist();

    NSString* nsTitle = [NSString stringWithCharacters:(const unichar *)title.unicode() length:(NSUInteger)title.length() ];
    NSString* nsDescription = [NSString stringWithCharacters:(const unichar *)description.unicode() length:(NSUInteger)description.length() ];

#ifdef LASTFM_USER_NOTIFICATIONS
    if ( [NSUserNotificationCenter class] )
    {
        NSUserNotification* userNotification = [[NSUserNotification alloc] init];

        [userNotification setTitle:nsTitle];
        [userNotification setSubtitle:nsDescription];

        [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:userNotification];
        [userNotification release];
    }
#endif
}


void
unicorn::Notify::notificationWasClicked()
{
    emit clicked();
}
