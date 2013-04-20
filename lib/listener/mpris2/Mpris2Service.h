/*
   Copyright (C) 2013 John Stamp <jstamp@mehercule.net>

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

#ifndef MPRIS2SERVICE_H
#define MPRIS2SERVICE_H

#include <QString>
#include <QVariantMap>

class QDBusInterface;

class Mpris2Service : public QObject
{
    Q_OBJECT
public:
    Mpris2Service( const QString& name, QObject * parent );
    ~Mpris2Service();

    QString name() const;
    QString identity() const;
    QString desktopEntry() const;

    QString artist() const;
    QString title() const;
    uint length() const;
    QString url() const;

signals:
    void stateChanged( const QString& );

private:
    QString m_state;
    QDBusInterface* m_propsIface;
    QVariantMap m_metadata;

    QVariant getProp( const QString& interface, const QString& prop ) const;

private slots:
    void propsChanged( const QString& interface,
            const QVariantMap& changedProperties,
            const QStringList& invalidatedProperties );
};

#endif
