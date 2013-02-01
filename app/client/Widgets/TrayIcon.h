#include <QSystemTrayIcon>
#include <QWheelEvent>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT;

public:
    TrayIcon( QObject* parent);

    bool event( QEvent* e);
};
