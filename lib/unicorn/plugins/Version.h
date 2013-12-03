/*
   Copyright 2013 Last.fm Ltd.
      - Primarily authored by Michael Coffey

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

#pragma once

#include <lib/DllExportMacro.h>

#include <QString>

namespace unicorn
{

class UNICORN_DLLEXPORT Version
{
public:
    Version();

    Version( unsigned int major, unsigned int minor = 0, unsigned int build = 0, unsigned int revision = 0 );

    QString toString() const;
    static Version fromString( const QString& string );

    bool operator !=( const Version& that ) const;
    bool operator ==( const Version& that ) const;
    bool operator <( const Version& that ) const;
    bool operator >( const Version& that ) const;

private:
    unsigned int m_major;
    unsigned int m_minor;
    unsigned int m_build;
    unsigned int m_revision;
};

}
