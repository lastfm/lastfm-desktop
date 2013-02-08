
#include <QStringList>

#include <../UnicornApplication.h>
#include <../UnicornSettings.h>

#include "Updater.h"

#include <AppKit/NSNibDeclarations.h>
#include <Foundation/Foundation.h>
#include <Foundation/NSURL.h>

#include <Sparkle/Sparkle.h>
#include <Sparkle/SUUpdater.h>

@interface UpdaterDelegate : NSObject {
}
- (void)updater:(SUUpdater *)updater willInstallUpdate:(SUAppcastItem *)update;
- (void)updaterWillRelaunchApplication:(SUUpdater *)updater;
@end

@implementation UpdaterDelegate

- (void)updater:(SUUpdater *)updater willInstallUpdate:(SUAppcastItem *)update
{
    Q_UNUSED(updater)
    Q_UNUSED(update)
}

- (void) updaterWillRelaunchApplication:(SUUpdater *)updater
{
    Q_UNUSED(updater)

    static_cast<unicorn::Application*>(qApp)->hideDockIcon( unicorn::Settings().value( SETTING_SHOW_WHERE, 0 ).toInt() == 2 );
}

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

        if ( qApp->arguments().count() > urlIndex && qApp->arguments()[urlIndex].startsWith( "http://" ) )
            [updater setFeedURL:[NSURL URLWithString: [NSString stringWithCharacters:(const unichar *)qApp->arguments()[urlIndex].unicode() length:(NSUInteger)qApp->arguments()[urlIndex].length() ]]];
        else
            setBetaUpdates( unicorn::Settings().value( SETTING_BETA_UPGRADES, false ).toBool() );
    }
    else
        setBetaUpdates( unicorn::Settings().value( SETTING_BETA_UPGRADES, false ).toBool() );

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

unicorn::Updater::~Updater()
{
    // do nothing - included here for header compatibility with windows version
}
