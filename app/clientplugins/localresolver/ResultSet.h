/***************************************************************************
 *   Copyright 2007-2008 Last.fm Ltd.                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/


#ifndef RESULT_SET_H
#define RESULT_SET_H

#include <QSet>

class ResultSet : public QSet<uint>
{
    // marks a special kind of result set which 
    // has come from an unsupported rql service name.
    // it behaves differently depending on the operation
    // so as not to ruin the whole query.  :)

protected:
    bool m_unsupported; 

public:
    ResultSet()
        :m_unsupported(false)
    {
    }

    ResultSet(const QSet<uint>& set)
        :QSet<uint>(set)
    {
    }

    ResultSet and(const ResultSet &other)
    {
        intersect(other);
        return *this;
    }

    ResultSet or(const ResultSet &other)
    {
        unite(other);
        return *this;
    }

    ResultSet and_not(const ResultSet &other)
    {
        subtract(other);
        return *this;
    }

};


struct UnsupportedResultSet : public ResultSet
{
    UnsupportedResultSet()
    {
        m_unsupported = true;
    }
};




#endif