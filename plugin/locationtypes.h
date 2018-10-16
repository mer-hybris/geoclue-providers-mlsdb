/*
    Copyright (C) 2016 Jolla Ltd.
    Contact: Chris Adams <chris.adams@jollamobile.com>

    This file is part of geoclue-mlsdb.

    geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#ifndef LOCATIONTYPES_H
#define LOCATIONTYPES_H

#include <QtCore/QtNumeric>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QMetaType>

class AccuracyData : public QSharedData
{
public:
    AccuracyData() : horizontal(qQNaN()), vertical(qQNaN()) { }
    AccuracyData(const AccuracyData &other)
    :   QSharedData(other), horizontal(other.horizontal), vertical(other.vertical)
    { }
    ~AccuracyData() { }

    double horizontal;
    double vertical;
};

class Accuracy
{
public:
    Accuracy() : d(new AccuracyData) { }
    Accuracy(const Accuracy &other) : d(other.d) { }

    inline double horizontal() const { return d->horizontal; }
    inline void setHorizontal(double accuracy) { d->horizontal = accuracy; }
    inline double vertical() const { return d->vertical; }
    inline void setVertical(double accuracy) { d->vertical = accuracy; }

private:
    QSharedDataPointer<AccuracyData> d;
};

class LocationData : public QSharedData
{
public:
    LocationData()
    :   timestamp(0), latitude(qQNaN()), longitude(qQNaN()), altitude(qQNaN()), speed(qQNaN()),
        direction(qQNaN()), climb(qQNaN())
    { }
    LocationData(const LocationData &other)
    :   QSharedData(other), timestamp(other.timestamp), latitude(other.latitude),
        longitude(other.longitude), altitude(other.altitude), speed(other.speed),
        direction(other.direction), climb(other.climb), accuracy(other.accuracy)
    { }
    ~LocationData() { }

    qint64 timestamp;
    double latitude;
    double longitude;
    double altitude;

    double speed;
    double direction;
    double climb;

    Accuracy accuracy;
};

class Location
{
public:
    Location() : d(new LocationData) { }
    Location(const Location &other) : d(other.d) { }

    inline qint64 timestamp() const { return d->timestamp; }
    inline void setTimestamp(qint64 timestamp) { d->timestamp = timestamp; }
    inline double latitude() const { return d->latitude; }
    inline void setLatitude(double latitude) { d->latitude = latitude; }
    inline double longitude() const { return d->longitude; }
    inline void setLongitude(double longitude) { d->longitude = longitude; }
    inline double altitude() const { return d->altitude; }
    inline void setAltitude(double altitude) { d->altitude = altitude; }
    inline double speed() const { return d->speed; }
    inline void setSpeed(double speed) { d->speed = speed; }
    inline double direction() const { return d->direction; }
    inline void setDirection(double direction) { d->direction = direction; }
    inline double climb() const { return d->climb; }
    inline void setClimb(double climb) { d->climb = climb; }
    inline Accuracy accuracy() const { return d->accuracy; }
    inline void setAccuracy(const Accuracy &accuracy) { d->accuracy = accuracy; }

private:
    QSharedDataPointer<LocationData> d;
};

Q_DECLARE_METATYPE(Accuracy)
Q_DECLARE_METATYPE(Location)

#endif // LOCATIONTYPES_H
