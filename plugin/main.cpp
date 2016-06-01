/*
    Copyright (C) 2016 Jolla Ltd.
    Contact: Chris Adams <chris.adams@jollamobile.com>

    This file is part of geoclue-mlsdb.

    geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#include <QtCore/QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtDBus/QDBusConnection>

#include "mlsdbprovider.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(QStringLiteral("geoclue.provider.mlsdb.debug=false\n"
                                                    "geoclue.provider.mlsdb.position.debug=false"));

    QCoreApplication a(argc, argv);
    MlsdbProvider provider;
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(QStringLiteral("org.freedesktop.Geoclue.Providers.Mlsdb")))
        qFatal("Failed to register service org.freedesktop.Geoclue.Providers.Mlsdb");
    if (!connection.registerObject(QStringLiteral("/org/freedesktop/Geoclue/Providers/Mlsdb"), &provider))
        qFatal("Failed to register object /org/freedesktop/Geoclue/Providers/Mlsdb");
    return a.exec();
}
