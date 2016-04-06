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
    MLSDB_CELL_TYPE_GSM = 1,
    MLSDB_CELL_TYPE_UMTS = 2,
    MLSDB_CELL_TYPE_OTHER = 3
};
QString stringForMlsdbCellType(MlsdbCellType type);

struct MlsdbUniqueCellId {
    MlsdbUniqueCellId() : m_cellId(0), m_locationCode(0), m_mcc(0), m_mnc(0) {}
    MlsdbUniqueCellId(MlsdbCellType cellType, quint32 cellId, quint32 locationAreaCode, quint16 mcc, quint16 mnc)
        : m_cellId((cellId << 4) | cellType)
        , m_locationCode(locationAreaCode)
        , m_mcc(mcc)
        , m_mnc(mnc) {}

    MlsdbUniqueCellId(const MlsdbUniqueCellId &other)
        : m_cellId(other.m_cellId)
        , m_locationCode(other.m_locationCode)
        , m_mcc(other.m_mcc)
        , m_mnc(other.m_mnc) {}

    bool operator==(const MlsdbUniqueCellId &other) const {
        return m_cellId == other.m_cellId
            && m_locationCode == other.m_locationCode
            && m_mcc == other.m_mcc
            && m_mnc == other.m_mnc;
    }
    bool operator<(const MlsdbUniqueCellId &other) const {
        if (m_cellId > other.m_cellId) return false;
        if (m_cellId < other.m_cellId) return true;
        if (m_locationCode > other.m_locationCode) return false;
        if (m_locationCode < other.m_locationCode) return true;
        if (m_mcc > other.m_mcc) return false;
        if (m_mcc < other.m_mcc) return true;
        return m_mnc < other.m_mnc;
    }

    QString toString() const {
        return QStringLiteral("type: %1, cellId: %2, locationCode: %3, mcc: %4, mnc: %5")
              .arg(stringForMlsdbCellType(cellType()))
              .arg(cellId()).arg(locationCode()).arg(mcc()).arg(mnc());
    }

    MlsdbCellType cellType() const { return static_cast<MlsdbCellType>(m_cellId & 0x0000000F); }
    quint32 cellId() const         { return (m_cellId & 0xFFFFFFF0) >> 4; }
    quint32 locationCode() const   { return m_locationCode; }
    quint16 mcc() const            { return m_mcc; }
    quint16 mnc() const            { return m_mnc; }

    quint32 m_cellId;       // low 4 bits encode the MlsdbCellType
    quint32 m_locationCode; // LAC or TAC
    quint16 m_mcc;          // 12 bits
    quint16 m_mnc;          // 8 or 12 bits
};
Q_DECLARE_TYPEINFO(MlsdbUniqueCellId, Q_PRIMITIVE_TYPE);

QDataStream &operator<<(QDataStream &out, const MlsdbUniqueCellId &cellId);
QDataStream &operator>>(QDataStream &in, MlsdbUniqueCellId &cellId);
uint qHash(const MlsdbUniqueCellId &key);

#endif // GEOCLUE_MLSDB_SERIALISATION_H
