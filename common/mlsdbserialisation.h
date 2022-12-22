/*
    Copyright (C) 2022 Jolla Ltd.
    Contact: Chris Adams <chris.adams@jolla.com>

    This file is part of geoclue-mlsdb.

    Geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#ifndef GEOCLUE_MLSDB_SERIALISATION_H
#define GEOCLUE_MLSDB_SERIALISATION_H

#include <QString>
#include <QtGlobal>

struct MlsdbCoords {
    float lat;
    float lon;
};
Q_DECLARE_TYPEINFO(MlsdbCoords, Q_PRIMITIVE_TYPE);

enum MlsdbCellType {
    MLSDB_CELL_TYPE_GSM = 0,
    MLSDB_CELL_TYPE_LTE = 1,
    MLSDB_CELL_TYPE_UMTS = 2,
    MLSDB_CELL_TYPE_OTHER = 3
};

quint64 getMlsdbUniqueCellId(MlsdbCellType cellType, quint32 cellId, quint32 locationAreaCode, quint16 mcc, quint16 mnc);

MlsdbCellType getCellType(quint64 id);
quint16 getCellMcc(quint64 id);
quint16 getCellMnc(quint64 id);
quint32 getCellArea(quint64 id);
quint32 getCellId(quint64 id);

#endif // GEOCLUE_MLSDB_SERIALISATION_H
