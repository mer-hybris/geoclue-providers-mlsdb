/*
    Copyright (C) 2016 Jolla Ltd.
    Contact: Chris Adams <chris.adams@jollamobile.com>

    This file is part of geoclue-mlsdb.

    Geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#include "mlsdbserialisation.h"

QDataStream &operator<<(QDataStream &out, const MlsdbCoords &coords)
{
    out << coords.lat << coords.lon;
    return out;
}

QDataStream &operator>>(QDataStream &in, MlsdbCoords &coords)
{
    in >> coords.lat >> coords.lon;
    return in;
}

QDataStream &operator<<(QDataStream &out, const MlsdbUniqueCellId &cellId)
{
    out << cellId.m_cellId << cellId.m_locationCode << cellId.m_mcc << cellId.m_mnc;
    return out;
}

QDataStream &operator>>(QDataStream &in, MlsdbUniqueCellId &cellId)
{
    in >> cellId.m_cellId >> cellId.m_locationCode >> cellId.m_mcc >> cellId.m_mnc;
    return in;
}

uint qHash(const MlsdbUniqueCellId &key)
{
    return key.m_cellId;
}

QString stringForMlsdbCellType(MlsdbCellType type)
{
    switch (type) {
        case MLSDB_CELL_TYPE_LTE: return QLatin1String("LTE");
        case MLSDB_CELL_TYPE_GSM: return QLatin1String("GSM");
        case MLSDB_CELL_TYPE_UMTS: return QLatin1String("UMTS");
        default: return QLatin1String("OTHER");
    }
}

