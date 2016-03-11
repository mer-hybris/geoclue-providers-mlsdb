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
