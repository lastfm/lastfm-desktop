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

#include <QCoreApplication>
#include <QFile>
#include <QStringList>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>
#include <QTimer>

#include <lastfm/Fingerprint.h>
#include <lastfm/FingerprintableSource.h>
#include <lastfm/Track.h>

#include "LAV_Source.h"
#include "Fingerprinter.h"


Fingerprinter::Fingerprinter( const lastfm::Track& track, QObject* parent )
    :QObject( parent ), m_fp( track ), m_fpSource( 0 )
{
    if ( m_fp.id().isNull() )
    {
        m_fpSource = new LAV_Source();

        if ( m_fpSource )
        {
            try
            {
                m_fp.generate( m_fpSource );
                connect( m_fp.submit(), SIGNAL(finished()), SLOT(onFingerprintSubmitted()) );
            }
            catch ( const lastfm::Fingerprint::Error& error )
            {
                qWarning() << "Fingerprint error: " << error;
                QTimer::singleShot(250, qApp, SLOT(quit()));
            }
        }
    }
    else
    {
        qDebug() << "Already Fingerprinted: " << m_fp.id();

#ifndef NDEBUG
        // This code will fetch the suggestions from the fingerprint id, one
        // day we might do something with this info, like offer corrections
        connect( m_fp.id().getSuggestions(), SIGNAL(finished()), SLOT(onGotSuggestions()) );
#else
        QTimer::singleShot(250, qApp, SLOT(quit()));
#endif
    }
}

Fingerprinter::~Fingerprinter()
{
    delete m_fpSource;
}

void
Fingerprinter::onFingerprintSubmitted()
{
    try
    {
        m_fp.decode( static_cast<QNetworkReply*>( sender() ) );
        qDebug() << "Fingerprint success: " << m_fp.id();
    }
    catch ( const lastfm::Fingerprint::Error& error )
    {
        qWarning() << "Fingerprint error: " << error;
        QTimer::singleShot(250, qApp, SLOT(quit()));
        return;
    }

#ifndef NDEBUG
    // This code will fetch the suggestions from the fingerprint id, one
    // day we might do something with this info, like offer corrections
    connect( m_fp.id().getSuggestions(), SIGNAL(finished()), SLOT(onGotSuggestions()) );
#else
    QTimer::singleShot(250, qApp, SLOT(quit()));
#endif

}

void
Fingerprinter::onGotSuggestions()
{
    QMap<float, Track> suggestions = lastfm::FingerprintId::getSuggestions( static_cast<QNetworkReply*>( sender() ) );
    qDebug() << suggestions;
    QTimer::singleShot(250, qApp, SLOT(quit()));
}
