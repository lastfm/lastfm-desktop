/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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
#ifdef __APPLE__
    // first to prevent compilation errors with Qt 4.5.0-beta1
    #include <Carbon/Carbon.h>
    #include <ApplicationServices/ApplicationServices.h>
    #include <QMainWindow>
    #include "UnicornApplicationDelegate.h"
    extern void qt_mac_set_menubar_icons( bool );
#elif defined WIN32
    #include <windows.h>
    #include <QAbstractEventDispatcher>
#endif

#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QLocale>
#include <QRegExp>
#include <QStyle>
#include <QTimer>
#include <QTranslator>
#include <QAction>

#include <lastfm/misc.h>
#include <lastfm/User.h>
#include <lastfm/InternetConnectionMonitor.h>
#include <lastfm/ws.h>
#include <lastfm/XmlQuery.h>

#include "PlayBus/Bus.h"
#include "dialogs/LoginContinueDialog.h"
#include "dialogs/LoginDialog.h"
#include "dialogs/UserManagerDialog.h"
#include "LoginProcess.h"
#include "QMessageBoxBuilder.h"
#include "SignalBlocker.h"
#include "UnicornCoreApplication.h"
#include "UnicornSettings.h"
#include "DesktopServices.h"
#include "UnicornApplication.h"




unicorn::Application::Application(const QString &id, int &argc, char **argv) throw( StubbornUserException )
                    : QtSingleApplication( id, argc, argv ),
                      m_logoutAtQuit( false ),
                      m_currentSession( new unicorn::Session ),
                      m_wizardRunning( true ),
                      m_icm( 0 )
{
    m_bus = new unicorn::Bus( this );

    qsrand( QDateTime::currentDateTime().toTime_t() + QCoreApplication::applicationPid() );

#ifdef Q_OS_MAC
    m_delegate = new unicorn::UnicornApplicationDelegate( this );
#endif
}

void
unicorn::Application::init()
{
#ifdef Q_OS_MAC
    addLibraryPath( applicationDirPath() + "/../plugins" );
#elif defined Q_OS_WIN
    addLibraryPath( applicationDirPath() + "/plugins" );
#endif

#ifdef Q_WS_MAC
    qt_mac_set_menubar_icons( false );
#endif

    CoreApplication::init();

    setupHotKeys();

#ifdef __APPLE__
    setGetURLEventHandler();
    AEEventHandlerUPP urlHandler = NewAEEventHandlerUPP( appleEventHandler );
    AEInstallEventHandler( kInternetEventClass, kAEGetURL, urlHandler, 0, false );

    setOpenApplicationEventHandler();
    AEEventHandlerUPP openHandler = NewAEEventHandlerUPP( appleEventHandler );
    AEInstallEventHandler( kCoreEventClass, kAEReopenApplication, openHandler, 0, false );
#endif

#ifdef Q_WS_MAC
#define CSS_PATH "/../Resources/"
#else
#define CSS_PATH "/"
#endif

    refreshStyleSheet();

    translate();

    m_icm = new lastfm::InternetConnectionMonitor( this );

    connect( m_icm, SIGNAL(up(QString)), this, SIGNAL(internetConnectionUp()));
    connect( m_icm, SIGNAL(down(QString)), this, SIGNAL(internetConnectionDown()));

    connect( m_bus, SIGNAL(wizardRunningQuery(QString)), SLOT(onWizardRunningQuery(QString)));
    connect( m_bus, SIGNAL(sessionQuery(QString)), SLOT(onBusSessionQuery(QString)));
    connect( m_bus, SIGNAL(sessionChanged(unicorn::Session)), SLOT(onBusSessionChanged(unicorn::Session)));
    connect( m_bus, SIGNAL(lovedStateChanged(bool)), SIGNAL(busLovedStateChanged(bool)));

    m_bus->board();

#ifdef __APPLE__
    setQuitOnLastWindowClosed( false );
#endif
}

void 
unicorn::Application::loadStyleSheet( QFile& file )
{
    file.open( QIODevice::ReadOnly );
    m_styleSheet += file.readAll();
    setStyleSheet( m_styleSheet );
}

void
unicorn::Application::initiateLogin( bool ) throw( StubbornUserException )
{
    Session* newSession = 0;

    if( m_bus->isWizardRunning() )
    {
        SignalBlocker( m_bus, SIGNAL( sessionChanged( const QMap<QString, QString>& ) ), -1 ).start();
    }
    else
    {
        QMap<QString, QString> sessionData = m_bus->getSessionData();

        //If the bus returns an empty session data, try to get the session from the last user logged in
        if ( ! ( sessionData.contains( "sessionKey" ) || sessionData.contains( "username" ) ) )
        {
            sessionData = Session::lastSessionData();
        }

        if ( sessionData.contains( "sessionKey" ) && sessionData.contains( "username" ) )
            newSession = new Session( sessionData[ "username" ], sessionData[ "sessionKey" ] );
    }

    if ( newSession )
    {
        changeSession( newSession );
    }
    else
    {
        SignalBlocker( m_bus, SIGNAL( sessionChanged( const QMap<QString, QString>& ) ), -1 ).start();

        QMap<QString, QString> sessionData = m_bus->getSessionData();

        if ( sessionData.contains( "sessionKey" ) && sessionData.contains( "username" ) )
        {
            newSession = new Session( sessionData[ "username" ], sessionData[ "sessionKey" ] );
            changeSession( newSession );
        }
        else
        {
            throw StubbornUserException();
        }
    }
}


void 
unicorn::Application::manageUsers()
{
    UserManagerDialog um;
    connect( &um, SIGNAL( rosterUpdated()), SIGNAL( rosterUpdated()));
    
    if( um.exec() )
    {
        QMap<QString, QString> lastSession = Session::lastSessionData();
        if ( lastSession.contains( "username" ) && lastSession.contains( "sessionKey" ) )
        {
            changeSession( lastSession[ "username" ], lastSession[ "sessionKey" ] );
        }
    }
}


void
unicorn::Application::translate()
{
    //Try to load the language set by the user and
    //if there wasn't any, then use the system language
    QString iso639 = AppSettings().value( "language", "" ).toString();
    if ( iso639.isEmpty() )
        iso639 = QLocale::system().name();

    qDebug() << "Locale: " << iso639;

    // set the default locale for the app which will be used by web services
    QLocale::setDefault( QLocale( iso639 ) );

    QString qmExt = iso639.left( 2 ) == "zh" ? iso639 : iso639.left( 2 );
    qDebug() << "Language ext: " << qmExt;

#ifdef Q_WS_MAC
    QDir const d = lastfm::dir::bundle().filePath( "Contents/Resources/qm" );
#elif defined(Q_OS_WIN)
    QDir const d = qApp->applicationDirPath() + "/i18n";
#elif defined(Q_OS_UNIX)
    QDir const d = QString( PREFIX ) + "/share/lastfm-scrobbler/i18n";
#endif

    //TODO need a unicorn/core/etc. translation, plus policy of no translations elsewhere or something!
    QTranslator* t1 = new QTranslator( this );
    t1->load( d.filePath( "lastfm_" + qmExt ) );

    QTranslator* t2 = new QTranslator( this );
    t2->load( d.filePath( "qt_" + qmExt ) );

    installTranslator( t1 );
    installTranslator( t2 );

#ifdef Q_OS_WIN
    QTranslator* t3 = new QTranslator( this );
    t3->load( d.filePath( "qtsparkle/" + qmExt ) );
    installTranslator( t3 );
#endif

#ifdef Q_OS_MAC
    macTranslate( qmExt );
#endif
}


unicorn::Application::~Application()
{
}


void
unicorn::Application::setWizardRunning( bool running )
{
    m_wizardRunning = running;
}

void
unicorn::Application::onWizardRunningQuery( const QString& uuid )
{
    qDebug() << "Is the Wizard running?";
    if ( m_wizardRunning )
    {
        m_bus->sendQueryResponse( uuid, "TRUE" );
    }
    else
    {
        m_bus->sendQueryResponse( uuid, "FALSE" );
    }

}

void 
unicorn::Application::onBusSessionQuery( const QString& uuid )
{
    QByteArray ba;
    QDataStream s( &ba, QIODevice::WriteOnly );
    QMap<QString, QString> sessionData;
    sessionData[ "username" ] = currentSession().user().name();
    sessionData[ "sessionKey" ] = currentSession().sessionKey();
    s << sessionData;
    m_bus->sendQueryResponse( uuid, ba );
}


void 
unicorn::Application::onBusSessionChanged( const unicorn::Session& session )
{
    unicorn::Session* newSession = new unicorn::Session( session.user().name(), session.sessionKey() );
    changeSession( newSession, false );
}

void
unicorn::Application::changeSession( const QString& username, const QString& sessionKey, bool announce )
{
    return changeSession( new unicorn::Session( username, sessionKey ), announce );
}

void
unicorn::Application::changeSession( Session* newSession, bool announce )
{
    if( m_currentSession && !m_wizardRunning &&  Settings().value( "changeSessionConfirmation", true ).toBool() )
    {
        bool dontAskAgain = false;
        int result = QMessageBoxBuilder( findMainWindow() ).setTitle( tr( "Changing User" ) )
           .setText( tr( "%1 will be logged into the Scrobbler and Last.fm Radio. "
                         "All music will now be scrobbled to this account. Do you want to continue?" )
                         .arg( newSession->user().name() ))
           .setIcon( QMessageBox::Information )
           .setButtons( QMessageBox::Yes | QMessageBox::Cancel )
           .dontAskAgain()
           .exec( &dontAskAgain );

        Settings().setValue( "changeSessionConfirmation", !dontAskAgain );
        if( result != QMessageBox::Yes )
            return;
    }

    disconnect( m_currentSession, SIGNAL(userInfoUpdated(lastfm::User)), this, SIGNAL(gotUserInfo(lastfm::User)) );
    connect( newSession, SIGNAL(userInfoUpdated(lastfm::User)), SIGNAL(gotUserInfo(lastfm::User)) );

    disconnect( m_currentSession, SIGNAL(sessionChanged(unicorn::Session)), this, SIGNAL(sessionChanged(unicorn::Session)) );
    connect( newSession, SIGNAL(sessionChanged(unicorn::Session)), SIGNAL(sessionChanged(unicorn::Session)) );

    delete m_currentSession;
    m_currentSession = newSession;

    lastfm::ws::Username = m_currentSession->user().name();
    lastfm::ws::SessionKey = m_currentSession->sessionKey();
    
    if( announce )
        m_bus->announceSessionChange( currentSession() );

    emit sessionChanged( currentSession() );
}

unicorn::Session&
unicorn::Application::currentSession() const
{
    return *m_currentSession;
}

void
unicorn::Application::sendBusLovedStateChanged( bool loved )
{
    QByteArray message = loved ? "LOVED=true" : "LOVED=false";
    m_bus->sendMessage(message);
}

void
unicorn::Application::refreshStyleSheet()
{
    m_styleSheet.clear();

    if ( m_cssFileName.isNull() )
    {
        // This is the first time we are loading the stylesheet

        if( !styleSheet().isEmpty() ) {
            m_cssFileName = QDir::currentPath() + QUrl( styleSheet() ).toLocalFile();
            m_cssDir = QFileInfo( m_cssFileName ).path();
        }

        if( styleSheet().isEmpty()) {
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
            m_cssFileName = applicationDirPath() + CSS_PATH + applicationName() + ".css";
            m_cssDir = applicationDirPath() + CSS_PATH;
#else
            m_cssFileName = QString( PREFIX ) + "/share/lastfm-scrobbler/" + applicationName() + ".css";
            m_cssDir = QString( PREFIX ) + "/share/lastfm-scrobbler/";
#endif
        }
    }

    if ( !m_cssFileName.isNull() )
    {
        QFile cssFile( m_cssFileName );
        cssFile.open( QIODevice::ReadOnly );
        m_styleSheet = cssFile.readAll();
        setStyleSheet( m_styleSheet );
        cssFile.close();
    }

    QRegExp rx( "@import\\s*\"([^\"]*)\";" );
    int pos = 0;
    while( (pos = rx.indexIn( m_styleSheet, pos )) != -1 ) {
        QFile f( m_cssDir + "/" +  rx.cap( 1 ));
        loadStyleSheet( f );
        pos += rx.matchedLength();
    }

//    QStyle* style = style();
//    style->set
//    setStyle( style );
}

void*
unicorn::Application::installHotKey( Qt::KeyboardModifiers modifiers, quint32 virtualKey, QObject* receiver, const char* slot )
{
    qDebug() << "Installing HotKey";
	quint32 id = m_hotKeyMap.size() + 1;
    m_hotKeyMap[id] = QPair<QObject*, const char*>( receiver, slot );
	
#ifdef __APPLE__
    EventHotKeyID hotKeyID;

    hotKeyID.signature='htk1';
    hotKeyID.id=id;

    UInt32 appleMod=0;
    if( modifiers.testFlag( Qt::ShiftModifier ))
        appleMod |= shiftKey;
    if( modifiers.testFlag( Qt::ControlModifier ))
        appleMod |= controlKey;
    if( modifiers.testFlag( Qt::AltModifier ))
        appleMod |= optionKey;
    if( modifiers.testFlag( Qt::MetaModifier ))
        appleMod |= cmdKey;

    EventHotKeyRef hkRef;

    RegisterEventHotKey( virtualKey, appleMod, hotKeyID, GetApplicationEventTarget(), 0, &hkRef );
    return hkRef;
#elif defined WIN32
	quint32 winMod = 0;
    if( modifiers.testFlag( Qt::ShiftModifier ))
        winMod |= MOD_SHIFT;
    if( modifiers.testFlag( Qt::ControlModifier ))
        winMod |= MOD_CONTROL;
    if( modifiers.testFlag( Qt::AltModifier ))
        winMod |= MOD_ALT;
    if( modifiers.testFlag( Qt::MetaModifier ))
        winMod |= MOD_WIN;
		
	RegisterHotKey( NULL, id, winMod, virtualKey);
    return reinterpret_cast<void*>(id);
#endif
}

void
unicorn::Application::unInstallHotKey( void* id )
{
#ifdef __APPLE__
    UnregisterEventHotKey( (EventHotKeyRef)id );
#elif defined WIN32
    UnregisterHotKey( NULL, (int)id );
#endif
}


void 
unicorn::Application::setupHotKeys()
{
#ifdef __APPLE__
    EventTypeSpec eventType;
    eventType.eventClass=kEventClassKeyboard;
    eventType.eventKind=kEventHotKeyPressed;

    using unicorn::Application;
    InstallApplicationEventHandler(&Application::hotkeyEventHandler, 1, &eventType, this, NULL);
#elif defined WIN32
    QAbstractEventDispatcher::instance()->setEventFilter( winEventFilter );
#endif
}

void 
unicorn::Application::onHotKeyEvent(quint32 id)
{
    QPair< QObject*, const char*> method = m_hotKeyMap[id];
    QObject* receiver = method.first;
    const char* slot = method.second;
    QTimer::singleShot( 0, receiver, slot );
}

QMainWindow*  
unicorn::Application::findMainWindow()
{
    QMainWindow* ret = 0;
    foreach (QWidget* w, qApp->topLevelWidgets())
        if ( (ret = qobject_cast<QMainWindow*>(w)) )
            break;

    return ret;
}

#ifdef __APPLE__
OSStatus /* static */
unicorn::Application::hotkeyEventHandler( EventHandlerCallRef, EventRef event, void* data )
{
    unicorn::Application* self = (unicorn::Application*)data;
    EventHotKeyID hkId;
    GetEventParameter( event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(hkId), NULL, &hkId);
    self->onHotKeyEvent( hkId.id );
    return noErr;
}


void
unicorn::Application::appleEventReceived( const QStringList& messages )
{
    emit messageReceived( messages );
}

pascal OSErr /* static */
#ifdef Q_OS_MAC64
unicorn::Application::appleEventHandler( const AppleEvent* e, AppleEvent*, void* )
#else
unicorn::Application::appleEventHandler( const AppleEvent* e, AppleEvent*, long )
#endif
{
    OSType id = typeWildCard;
    AEGetAttributePtr( e, keyEventIDAttr, typeType, 0, &id, sizeof(id), 0 );
 
    if ( id  == kAEQuitApplication )
    {
        qApp->quit();
        return noErr;
    }
    else if ( id == kAEGetURL )
    {
        OSErr err = noErr;
        Size actualSize = 0;
        DescType descType = typeChar;

        if ( (err = AESizeOfParam( e, keyDirectObject, &descType, &actualSize)) == noErr )
        {
            if ( 0 != actualSize )
            {
                // make a buffer (Qt style)
                QByteArray bUrl;
                bUrl.resize(actualSize);

                err = AEGetParamPtr(e,
                                    keyDirectObject,
                                    typeChar,
                                    0,
                                    bUrl.data(),
                                    actualSize,
                                    &actualSize);

                qobject_cast<unicorn::Application*>(qApp)->appleEventReceived( QStringList() << bUrl );
            }
        }

        return noErr;
    }
    else
    {
        AEAddressDesc descList;

        OSErr ret;
        ret = AEGetParamDesc( e, keyAEPropData, typeAEList, &descList );
        long count = 0;
        ret = AECountItems( &descList, &count );
        if( ret != noErr )
            count = 0;

        QStringList args;
        for( int i = 1; i <= count; ++i ) {
            AEAddressDesc desc;
            AEGetNthDesc( &descList, i, typeChar, NULL, &desc );
            if( ret == noErr ) {
                unsigned int size = AEGetDescDataSize( &desc );
                char data[size + 1];
                data[ size ] = 0;
                ret = AEGetDescData( &desc, data, size );
                args << QString::fromUtf8( data );
            }
        }

        qobject_cast<unicorn::Application*>(qApp)->appleEventReceived( args );
        return noErr;
    }

    return unimpErr;
}
#endif

#ifdef WIN32
bool /* static */
unicorn::Application::winEventFilter ( void* message )
{
	MSG* msg = (MSG*)message;
	if( msg->message == WM_HOTKEY )
	{
		qDebug() << "Filtered WM_HOTKEY";
        qobject_cast<unicorn::Application*>(qApp)->onHotKeyEvent( msg->wParam );
		return true;
	}
	return false;
}
#endif

void
unicorn::Application::restart()
{

#ifdef Q_OS_WIN
    QStringList args;
    args << "--new"; // force a new instance of the app, even if one is running
    qDebug() << applicationFilePath();
    QProcess::startDetached( applicationFilePath(), args );
#endif

#ifdef Q_OS_MAC
    // In Mac OS the full path of aplication binary is:
    //    <base-path>/myApp.app/Contents/MacOS/myApp
    QStringList args;
    args << "-b";
    args << "fm.last.Scrobbler";
    args << "-n"; // open a new instance even if one is running
    args << "--args";
    args << "--new"; // force a new instance of the app, even if one is running
    qDebug() << QProcess::startDetached( "open", args );
#endif

    // Terminate current instance:
    exit(0); // Exit gracefully by terminating the myApp instance
}

bool
unicorn::Application::isInternetConnectionUp() const
{
    return m_icm->isUp();
}

#ifdef Q_OS_MAC
void
unicorn::Application::showDockIcon( bool show )
{
    m_delegate->showDockIcon( show );
}
#endif

