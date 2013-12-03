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
#ifndef DIAGNOSTICS_DIALOG_H
#define DIAGNOSTICS_DIALOG_H

#include <lastfm/Track.h>

#include <QDialog>
#include <QLabel>
#include <QDateTime>
#include <QPointer>
#include <QProcess> //Qt enums
#include <QFile>

namespace Ui { class DiagnosticsDialog; }

class DiagnosticsDialog : public QDialog
{
    Q_OBJECT
public:
    DiagnosticsDialog( QWidget *parent = 0 );
    ~DiagnosticsDialog();
    
public slots:
    void fingerprinted( const Track& );
    void scrobbleActivity( int );

private slots:
    void onScrobblePointReached();
    
private:
	void scrobbleIPod( bool isManual = false );
	QString diagnosticInformation();

private slots:
	void onScrobbleIPodClicked();
	void onSendLogsClicked();
	void poll();

private:
    Ui::DiagnosticsDialog* ui;
    class DelayedLabelText* m_delay;
    QFile* m_ipod_log;
};


#include <QTimer>
class DelayedLabelText : public QObject
{
    Q_OBJECT
    
    QList<QString> texts;
    QTimer m_timer;
    
public:
    DelayedLabelText( QLabel* parent ) : QObject( parent )
    {
        m_timer.setInterval( 2000 );
        connect( &m_timer, SIGNAL(timeout()), SLOT(timeout()) );
    }
    
    void add( QString text )
    {       
        QLabel* label = static_cast<QLabel*>(parent());

        if (m_timer.isActive()) {
            if (texts.isEmpty() || texts.last() != text)
                texts += text;
            return;
        }
        
        label->setText( text );
        m_timer.start();
    }
    
private slots:
    void timeout()
    {
        if (texts.size())
            static_cast<QLabel*>(parent())->setText( texts.takeFirst() );
        if (texts.isEmpty())
            m_timer.stop();
    }
};


#endif //DIAGNOSTICS_DIALOG_H
