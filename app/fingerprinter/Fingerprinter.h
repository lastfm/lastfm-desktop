/*
   Copyright 2009 Last.fm Ltd. 
   Copyright 2009 John Stamp <jstamp@users.sourceforge.net>

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QObject>

#include <lastfm/Track.h>
#include <lastfm/Fingerprint.h>

namespace lastfm { class FingerprintableSource; }

class Fingerprinter : public QObject
{
    Q_OBJECT
public:
    explicit Fingerprinter( const lastfm::Track& track, QObject* parent = 0 );
    ~Fingerprinter();

private slots:
    void onFingerprintSubmitted();
    void onGotSuggestions();

private:
    lastfm::Fingerprint m_fp;
    lastfm::FingerprintableSource* m_fpSource;
    lastfm::Track m_track;
};


