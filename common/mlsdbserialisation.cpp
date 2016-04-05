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

quint64 composeMlsdbCellId(MlsdbCellType cellType, quint32 locationAreaCode, quint32 cellId)
{
    // A valid CID ranges from 0 to 65535 (2^16-1) on GSM and CDMA networks
    // and from 0 to 268435455 (2^28-1) on UMTS and LTE networks.
    // Hence, we have 4 bits we can use to store the cellType,
    // within a quint32.
    // Then we store the location area code above that, forming a single 64 bit number.
    quint64 retn = 0;
    retn |= static_cast<quint64>(cellType & 0xF);
    retn |= (static_cast<quint64>(cellId) << 4);
    retn |= (static_cast<quint64>(locationAreaCode) << 32);
    return retn;
}

void decomposeMlsdbCellId(quint64 composed, MlsdbCellType *cellType, quint32 *locationAreaCode, quint32 *cellId)
{
    *cellType = static_cast<MlsdbCellType>(composed & 0x000000000000000F);
    *cellId = (static_cast<quint32>(composed & 0x00000000FFFFFFF0) >> 4);
    *locationAreaCode = static_cast<quint32>((composed & 0xFFFFFFFF00000000) >> 32);
}

QString stringForMlsdbCellType(MlsdbCellType type)
{
    switch (type) {
        case MLSDB_CELL_TYPE_LTE: return QLatin1String("LTE");
        case MLSDB_CELL_TYPE_GSM_WCDMA: return QLatin1String("GSM_WCDMA");
        case MLSDB_CELL_TYPE_UMTS: return QLatin1String("UMTS");
        default: return QLatin1String("OTHER");
    }
}
