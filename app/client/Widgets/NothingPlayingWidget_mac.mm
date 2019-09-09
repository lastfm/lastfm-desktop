

#include "NothingPlayingWidget.h"

void
NothingPlayingWidget::oniTunesClicked( const QString& playerAppId )
{
    NSString *bundleId = [NSString stringWithUTF8String:playerAppId.toStdString().c_str()];
    // launch iTunes or Music  
    [[NSWorkspace sharedWorkspace] launchAppWithBundleIdentifier:bundleId options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifier:NULL];
}

