#include <Widgets/TrayIcon.h>

#include <Services/RadioService.h>

TrayIcon::TrayIcon( QObject* parent )
{

}

bool
TrayIcon::event( QEvent* e )
{
    // Beginning with Qt 4.3, QSystemTrayIcon supports wheel events, but only
    // on X11. Let's make it adjust the volume.
    if ( e->type() == QEvent::Wheel )
    {
        if ( ((QWheelEvent*)e)->delta() > 0 )
        {
            RadioService::instance().volumeUp();
        } else {
            RadioService::instance().volumeDown();
        }

        return true;
    }
    return QSystemTrayIcon::event( e );
}
