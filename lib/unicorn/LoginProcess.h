/*
   Copyright 2010-2013 Last.fm Ltd.
      - Primarily authored by William Viana Soares and Michael Coffey

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

#ifndef LOGIN_PROCESS_H
#define LOGIN_PROCESS_H

#include <QObject>
#include <QPointer>

#include <lastfm/ws.h>

#include "UnicornSession.h"

#include "lib/DllExportMacro.h"

namespace unicorn
{
/**
 * This class encapsulates the whole login process.
 *
 * Call the authenticate function to start the login process
 * and connect to the gotSession signal to be notified when
 * the process finishes.
 */
class UNICORN_DLLEXPORT LoginProcess : public QObject
{
    Q_OBJECT
public:
    LoginProcess( QObject* parent = 0 );
    ~LoginProcess();

private:
    void handleError( const lastfm::XmlQuery& lfm );

signals:
    void authUrlChanged( const QString& authUrl );

public slots:
    void authenticate();
    void getSession();

private slots:
    void onGotToken();
    void onGotSession();

private: 
    QString m_token;
};

}// namespace unicorn

#endif // LOGIN_PROCESS_H
