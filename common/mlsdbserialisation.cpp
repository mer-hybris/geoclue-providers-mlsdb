/*
    Copyright (C) 2022 Jolla Ltd.
    Contact: Daniel Suni <daniel.suni@jolla.com>

    This file is part of geoclue-mlsdb.

    Geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#include "mlsdbserialisation.h"
#include "mccmapping.h"

quint64 getMlsdbUniqueCellId(MlsdbCellType cellType, quint32 cellId, quint32 locationAreaCode, quint16 mcc, quint16 mnc)
{
    quint64 id, temp;
    quint16 i = 0;
    // mcc 10 bits, mnc 10 bits, lAC 16 bits, cId 28 bits
    if (mcc > 1023 || mnc > 1023 || locationAreaCode > 65534 || cellId > 268435455) {
        fprintf(stderr, "WARNING: Received a value that was too big. mcc was %d, max 1023, mnc was %d, max 1023\n", mcc, mnc);
        fprintf(stderr, "locationAreaCode was %d, max 65534, cellId was %d, max 268435455\n", locationAreaCode, cellId);
        return 0;
    }
    while (mccMap[i] < mcc) {
        ++i;
    }
    if (mccMap[i] != mcc) {
        fprintf(stderr, "WARNING: Received an mcc (%d) which is not mapped.", mcc);
        return 0;
    }
    id = i;
    id <<= 56;
    temp = mnc;
    temp <<= 46;
    id |= temp;
    temp = locationAreaCode;
    temp <<= 30;
    id |= temp;
    temp = cellId;
    temp <<= 2;
    id |= temp;
    id += cellType;
    return id;
}

MlsdbCellType getCellType(quint64 id)
{
    return (MlsdbCellType)(id & 3);
}

quint16 getCellMcc(quint64 id)
{
    return (quint16)mccMap[id >> 56];
}

quint16 getCellMnc(quint64 id)
{
    return (quint16)((id >> 46) & 0x3FF);
}

quint32 getCellArea(quint64 id)
{
    return (quint32)((id >> 30) & 0xFFFF);
}

quint32 getCellId(quint64 id)
{
    return (quint32)((id >> 2) & 0xFFFFFFF);
}
