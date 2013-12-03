/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Jono Cole and Michael Coffey

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

#ifndef SCROBBLECONFIRMATIONDIALOG_H
#define SCROBBLECONFIRMATIONDIALOG_H

#include <QDialog>

#include <lastfm/Track.h>

#include "lib/DllExportMacro.h"

namespace Ui { class ScrobbleConfirmationDialog; }

class ScrobblesModel;

class UNICORN_DLLEXPORT ScrobbleConfirmationDialog : public QDialog
{
    Q_OBJECT
public:
    ScrobbleConfirmationDialog( const QList<lastfm::Track>& tracks, QWidget* parent = 0 );
    ~ScrobbleConfirmationDialog();

    QList<lastfm::Track> tracksToScrobble() const;

    void addTracks( const QList<lastfm::Track>& tracks );

    const QStringList& files() const;
    void addFiles( const QStringList& files );

    void setReadOnly();

    bool autoScrobble() const;

private slots:
    void toggleSelection();

private:
    Ui::ScrobbleConfirmationDialog* ui;

    ScrobblesModel* m_scrobblesModel;
    bool m_toggled;
    QStringList m_files;
};

#endif // SCROBBLECONFIRMATIONDIALOG_H
