/*
    Copyright (C) 2016 Jolla Ltd.
    Contact: Chris Adams <chris.adams@jollamobile.com>

    This file is part of geoclue-mlsdb.

    Geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#ifndef GEOCLUE_MLSDB_SERIALISATION_H
#define GEOCLUE_MLSDB_SERIALISATION_H

#include <QDataStream>

struct MlsdbCoords {
    double lat;
    double lon;
};
Q_DECLARE_TYPEINFO(MlsdbCoords, Q_PRIMITIVE_TYPE);

QDataStream &operator<<(QDataStream &out, const MlsdbCoords &coords);
QDataStream &operator>>(QDataStream &in, MlsdbCoords &coords);

#endif // GEOCLUE_MLSDB_SERIALISATION_H
