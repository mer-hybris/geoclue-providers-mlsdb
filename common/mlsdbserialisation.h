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

enum MlsdbCellType {
    MLSDB_CELL_TYPE_LTE = 0,
    MLSDB_CELL_TYPE_GSM_WCDMA = 1,
    MLSDB_CELL_TYPE_UMTS = 2,
    MLSDB_CELL_TYPE_OTHER = 3
};
QString stringForMlsdbCellType(MlsdbCellType type);
quint64 composeMlsdbCellId(MlsdbCellType cellType, quint32 locationAreaCode, quint32 cellId);
void decomposeMlsdbCellId(quint64 composed, MlsdbCellType *cellType, quint32 *locationAreaCode, quint32 *cellId);

#endif // GEOCLUE_MLSDB_SERIALISATION_H
