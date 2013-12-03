/*
   Copyright 2004 Gareth Simpson <iscrobbler@xurble.org>
   Copyright 2004-2009 Last.fm Ltd. 

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

// Don't allow windows.h to include old winsock.h because it conflicts with
// winsock2.h included by the ScrobSubmitter
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include "iTunesVisualAPI/iTunesVisualAPI.h"
#include <time.h>
#include <string>

#if TARGET_OS_WIN32
#include <Gdiplus.h>
#endif // TARGET_OS_WIN32

#if TARGET_OS_WIN32
#define kTVisualPluginName     L"Last.fm AudioScrobbler"
#else
#define kTVisualPluginName     CFSTR("Last.fm AudioScrobbler")
#endif

#define	kTVisualPluginCreator			'hook'

#define	kTVisualPluginMajorVersion		6
#define	kTVisualPluginMinorVersion		0
#define	kTVisualPluginReleaseStage		5
#define	kTVisualPluginNonFinalRelease	4

struct VisualPluginData;

#if TARGET_OS_MAC
#import <Cocoa/Cocoa.h>

// "namespace" our ObjC classname to avoid load conflicts between multiple visualizer plugins
#define VisualView		ComAppleExample_VisualView
#define GLVisualView	ComAppleExample_GLVisualView

@class VisualView;
@class GLVisualView;

OSStatus	ConfigureVisual( VisualPluginData * visualPluginData );

void notificationCallback( CFNotificationCenterRef,
                          void*,
                          CFStringRef,
                          const void*,
                          CFDictionaryRef info );

#endif



#define kInfoTimeOutInSeconds		10							// draw info/artwork for N seconds when it changes or playback starts
#define kPlayingPulseRateInHz		10							// when iTunes is playing, draw N times a second
#define kStoppedPulseRateInHz		5							// when iTunes is not playing, draw N times a second

struct VisualPluginData
{
	void *				appCookie;
	ITAppProcPtr		appProc;
    
#if TARGET_OS_MAC
	NSView*				destView;
	NSRect				destRect;
	NSImage *			currentArtwork;
#else
	HWND				destView;
	RECT				destRect;
	Gdiplus::Bitmap* 	currentArtwork;
	long int			lastDrawTime;
#endif
	OptionBits			destOptions;
    
	RenderVisualData	renderData;
	UInt32				renderTimeStampID;
	
	ITTrackInfo			trackInfo;
	ITStreamInfo		streamInfo;
    
	// Plugin-specific data
    
	Boolean				playing;								// is iTunes currently playing audio?
	Boolean				padding[3];
    
	time_t				drawInfoTimeOut;						// when should we stop showing info/artwork?
    
	UInt8				minLevel[kVisualMaxDataChannels];		// 0-128
	UInt8				maxLevel[kVisualMaxDataChannels];		// 0-128
};
typedef struct VisualPluginData VisualPluginData;

void		GetVisualName( ITUniStr255 name );
OptionBits	GetVisualOptions( void );
OSStatus	RegisterVisualPlugin( PluginMessageInfo * messageInfo );

void		ProcessRenderData( VisualPluginData * visualPluginData, UInt32 timeStampID, const RenderVisualData * renderData );
void		ResetRenderData( VisualPluginData * visualPluginData );
void		UpdateTrackInfo( VisualPluginData * visualPluginData, ITTrackInfo * trackInfo, ITStreamInfo * streamInfo );
void		UpdatePulseRate( VisualPluginData * visualPluginData, UInt32 * ioPulseRate );

void		PulseVisual( VisualPluginData * visualPluginData, UInt32 timeStampID, const RenderVisualData * renderData, UInt32 * ioPulseRate );
void		InvalidateVisual( VisualPluginData * visualPluginData );

std::string	GetVersionString();
OSStatus	VisualPluginHandler( OSType m, VisualPluginMessageInfo*, void* );
