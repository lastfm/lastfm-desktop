/*
 * Copyright 2012  Alex Merry <alex.merry@kdemail.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DBUSABSTRACTADAPTOR_H
#define DBUSABSTRACTADAPTOR_H

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusConnection>

#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

class PropertiesChangedAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.Properties")

public:
    explicit PropertiesChangedAdaptor( QObject *parent );
    void emitPropertiesChanged( const QString &interface,
                                const QVariantMap &updatedProperties,
                                const QStringList &invalidatedProperties );

Q_SIGNALS:
    void propertiesChanged( const QString &interface,
                            const QVariantMap &updatedProperties,
                            const QStringList &invalidatedProperties );
};

/**
 * Hack for property notification support
 */
class DBusAbstractAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT

public:
    explicit DBusAbstractAdaptor( QObject *parent );

    // These are hackish methods that are necessary because
    // of the way QtDBus is implemented; it is impossible to
    // find out what bus or path this adaptor is at, and adding
    // another adaptor for the properties interface prevents
    // Qt's internal properties implementation from working
    QDBusConnection connection() const;
    void setConnection( const QDBusConnection &conn );
    QString dBusPath() const;
    void setDBusPath( const QString &path );

protected:
    void signalPropertyChange( const QString &property, const QVariant &value );
    void signalPropertyChange( const QString &property );

private Q_SLOTS:
    void _m_emitPropertiesChanged();

private:
    QStringList     m_invalidatedProperties;
    QVariantMap     m_updatedProperties;
    QString         m_path;
    QDBusConnection m_connection;
};

#endif // DBUSABSTRACTADAPTOR_H
