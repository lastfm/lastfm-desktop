
#include <QComboBox>

#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/QMessageBoxBuilder.h"

#include "../Application.h"

#include "ui_GeneralSettingsWidget.h"
#include "GeneralSettingsWidget.h"

GeneralSettingsWidget::GeneralSettingsWidget( QWidget* parent )
    :SettingsWidget( parent ),
      ui( new Ui::GeneralSettingsWidget )
{
    ui->setupUi( this );

    populateLanguages();
    connect( ui->languages, SIGNAL( currentIndexChanged( int ) ), SLOT( onSettingsChanged() ) );

    ui->notifications->setChecked( unicorn::Settings().value( SETTING_NOTIFICATIONS, ui->notifications->isChecked() ).toBool() );
    ui->lastRadio->setChecked( unicorn::Settings().value( SETTING_LAST_RADIO, ui->lastRadio->isChecked() ).toBool() );
    ui->sendCrashReports->setChecked( unicorn::Settings().value( SETTING_SEND_CRASH_REPORTS, ui->sendCrashReports->isChecked() ).toBool() );
    ui->beta->setChecked( unicorn::Settings().value( SETTING_BETA_UPGRADES, false ).toBool() );

    connect( ui->notifications, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
    connect( ui->lastRadio, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
    connect( ui->sendCrashReports, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
    connect( ui->beta, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );

#if !defined( Q_OS_WIN ) && !defined( Q_OS_MAC )
    ui->beta->hide(); // only have a beta update setting in mac and windows
#endif

#ifdef Q_OS_MAC
    ui->showAs->hide();

    ui->mediaKeys->setChecked( unicorn::Settings().value( "mediaKeys", true ).toBool() );

    connect( ui->mediaKeys, SIGNAL(stateChanged(int)), SLOT(onSettingsChanged()) );

    ui->showWhere->addItem( tr("Menu bar and dock icons") );
    ui->showWhere->addItem( tr("Only dock icon (hide menu bar icon)") );
    ui->showWhere->addItem( tr("Only menu bar icon (hide dock icon)") );

    int showWhereIndex = unicorn::Settings().value( SETTING_SHOW_WHERE, -1 ).toInt();

    if ( showWhereIndex == -1 )
    {
        bool showAs = unicorn::Settings().value( SETTING_SHOW_AS, true ).toBool();
        ui->showWhere->setCurrentIndex( showAs ? 0 : 1 );
        unicorn::Settings().setValue( SETTING_SHOW_WHERE, ui->showWhere->currentIndex() );
    }
    else
        ui->showWhere->setCurrentIndex( showWhereIndex );

    connect( ui->showWhere, SIGNAL(currentIndexChanged(int)), SLOT(onSettingsChanged()) );

#else
    ui->showWhere->hide();
    ui->showWhereLabel->hide();
    ui->mediaKeys->hide();

    ui->showAs->setChecked( unicorn::Settings().value( SETTING_SHOW_AS, ui->showAs->isChecked() ).toBool() );
    connect( ui->showAs, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
#endif

#ifndef Q_WS_X11
    ui->launch->setChecked( unicorn::AppSettings( OLDE_PLUGIN_SETTINGS ).value( SETTING_LAUNCH_ITUNES, ui->launch->isChecked() ).toBool() );
    ui->updates->setChecked( unicorn::Settings().value( SETTING_CHECK_UPDATES, ui->updates->isChecked() ).toBool() );
    connect( ui->launch, SIGNAL(stateChanged(int) ), SLOT( onSettingsChanged() ) );
    connect( ui->updates, SIGNAL(stateChanged(int)), SLOT( onSettingsChanged() ) );
#else
    ui->launch->hide();
    ui->updates->hide();
#endif
}

void
GeneralSettingsWidget::populateLanguages()
{
    ui->languages->addItem( tr( "System Language" ), "" );
    ui->languages->addItem( "English", QLocale( QLocale::English, QLocale::UnitedKingdom ).name() );
    ui->languages->addItem( QString::fromUtf8( "français" ), QLocale( QLocale::French ).name() );
    ui->languages->addItem( "Italiano", QLocale( QLocale::Italian ).name() );
    ui->languages->addItem( "Deutsch", QLocale( QLocale::German ).name() );
    ui->languages->addItem( QString::fromUtf8( "Español" ), QLocale( QLocale::Spanish ).name() );
    ui->languages->addItem( QString::fromUtf8( "Português" ), QLocale( QLocale::Portuguese, QLocale::Brazil ).name() );
    ui->languages->addItem( "Polski", QLocale( QLocale::Polish ).name() );
    ui->languages->addItem( "Svenska", QLocale( QLocale::Swedish ).name());
    ui->languages->addItem( QString::fromUtf8( "Türkçe" ), QLocale( QLocale::Turkish ).name() );
    ui->languages->addItem( QString::fromUtf8( "� уccкий" ), QLocale( QLocale::Russian ).name() );
    ui->languages->addItem( QString::fromUtf8( "简体中文" ), QLocale( QLocale::Chinese, QLocale::China ).name() );
    ui->languages->addItem( QString::fromUtf8( "日本語" ), QLocale( QLocale::Japanese ).name());

    QString currLanguage = unicorn::AppSettings().value( "language", "" ).toString();
    int index = ui->languages->findData( currLanguage );
    if ( index != -1 )
    {
        ui->languages->setCurrentIndex( index );
    }
}

void
GeneralSettingsWidget::saveSettings()
{
    qDebug() << "has unsaved changes?" << hasUnsavedChanges();
    if ( hasUnsavedChanges() )
    {
        bool restartNeeded = false;

        int currIndex = ui->languages->currentIndex();
        QString currLanguage = ui->languages->itemData( currIndex ).toString();

        if ( unicorn::AppSettings().value( "language", "" ) != currLanguage )
        {
            if ( currLanguage == ""  )
                QLocale::setDefault( QLocale::system() );
            else
                QLocale::setDefault( QLocale( currLanguage ) );

            unicorn::AppSettings().setValue( "language", currLanguage );

#ifdef Q_OS_MAC
            aApp->translate();
#endif

            restartNeeded = true;
        }

        // setting is for the 'Client' aplication for compatibility with old media player plugins
        unicorn::AppSettings( OLDE_PLUGIN_SETTINGS ).setValue( SETTING_LAUNCH_ITUNES, ui->launch->isChecked() );

        unicorn::Settings().setValue( SETTING_NOTIFICATIONS, ui->notifications->isChecked() );
        unicorn::Settings().setValue( SETTING_LAST_RADIO, ui->lastRadio->isChecked() );
        unicorn::Settings().setValue( SETTING_SEND_CRASH_REPORTS, ui->sendCrashReports->isChecked() );
        unicorn::Settings().setValue( SETTING_CHECK_UPDATES, ui->updates->isChecked() );
        unicorn::Settings().setValue( SETTING_BETA_UPGRADES, ui->beta->isChecked() );

        aApp->setBetaUpdates( ui->beta->isChecked() );

#ifdef Q_OS_MAC
        unicorn::Settings().setValue( "mediaKeys", ui->mediaKeys->isChecked() );
        aApp->setMediaKeysEnabled( ui->mediaKeys->isChecked() );

        int showWhereIndex = unicorn::Settings().value( SETTING_SHOW_WHERE, -1 ).toInt();
        bool showAs = ui->showWhere->currentIndex() != 1;

        unicorn::Settings().setValue( SETTING_SHOW_WHERE, ui->showWhere->currentIndex() );
        unicorn::Settings().setValue( SETTING_SHOW_AS, showAs );

        aApp->showAs( showAs );
        aApp->hideDockIcon( ui->showWhere->currentIndex() == 2 );

        if ( ( showWhereIndex != 2 && ui->showWhere->currentIndex() == 2 )
             || (showWhereIndex == 2 && ui->showWhere->currentIndex() != 2) )
            restartNeeded = true;
#else
        unicorn::Settings().setValue( SETTING_SHOW_AS, ui->showAs->isChecked() );
        aApp->showAs( ui->showAs->isChecked() );
#endif

        onSettingsSaved();

        if ( restartNeeded )
        {

            int button = QMessageBoxBuilder( this )
                            .setIcon( QMessageBox::Question )
                            .setTitle( tr( "Restart now?" ) )
                            .setText( tr( "An application restart is required for the change to take effect. Would you like to restart now?" ) )
                            .setButtons( QMessageBox::Yes | QMessageBox::No )
                            .exec();

            if ( button == QMessageBox::Yes )
                aApp->restart();

        }
    }
}
