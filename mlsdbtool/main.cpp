/*
    Copyright (C) 2016 Jolla Ltd.
    Contact: Chris Adams <chris.adams@jollamobile.com>

    This file is part of geoclue-mlsdb.

    Geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#include <QCoreApplication>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QSet>
#include <QVariant>
#include <QDataStream>
#include <QFile>
#include <QDir>

#include <stdio.h>
#include <stdlib.h>

#define USE_CONCURRENT 0
#if USE_CONCURRENT
#define USE_STD_CONCURRENT 1
#define THREADCOUNT 3
#if USE_STD_CONCURRENT
#include <thread>
#include <future>
#else
#include <qtconcurrentrun.h>
#endif
#endif

#include "mlsdbserialisation.h"

struct BoundingBox {
    // usually zero, except in cases where the country crosses meridian or pole
    double latShift;
    double lonShift;

    // bounding box lower-left/upper-right corners
    // note that these are post-shift values!
    MlsdbCoords lowerLeft;
    MlsdbCoords upperRight;

    bool contains(const MlsdbCoords &c) const {
        if (Q_UNLIKELY(latShift != 0.0 || lonShift != 0.0)) {
            return (c.lat+latShift) >= lowerLeft.lat && (c.lat+latShift) <= upperRight.lat
                && (c.lon+lonShift) >= lowerLeft.lon && (c.lon+lonShift) <= upperRight.lon;
        }

        return c.lat >= lowerLeft.lat && c.lat <= upperRight.lat
            && c.lon >= lowerLeft.lon && c.lon <= upperRight.lon;
    }
};
Q_DECLARE_TYPEINFO(BoundingBox, Q_PRIMITIVE_TYPE);

QDataStream &operator<<(QDataStream &out, const BoundingBox &bbox)
{
    out << bbox.latShift << bbox.lonShift << bbox.lowerLeft << bbox.upperRight;
    return out;
}

QDataStream &operator>>(QDataStream &in, BoundingBox &bbox)
{
    in >> bbox.latShift >> bbox.lonShift >> bbox.lowerLeft >> bbox.upperRight;
    return in;
}

// These are approximate only!
QMap<QString, BoundingBox> countryBoundingBoxes()
{
    QMap<QString, BoundingBox> retn;

// Europe

    BoundingBox aland_islands_bb;
    aland_islands_bb.latShift = 0.0;
    aland_islands_bb.lonShift = 0.0;
    aland_islands_bb.lowerLeft.lat = 59.736;
    aland_islands_bb.lowerLeft.lon = 19.263;
    aland_islands_bb.upperRight.lat = 60.666;
    aland_islands_bb.upperRight.lon = 21.324;
    retn.insert(QLatin1String("Aland_Islands"), aland_islands_bb);

    BoundingBox albania_bb;
    albania_bb.latShift = 0.0;
    albania_bb.lonShift = 0.0;
    albania_bb.lowerLeft.lat = 39.625;
    albania_bb.lowerLeft.lon = 19.304;
    albania_bb.upperRight.lat = 42.688;
    albania_bb.upperRight.lon = 21.020;
    retn.insert(QLatin1String("Albania"), albania_bb);

    BoundingBox andorra_bb;
    andorra_bb.latShift = 0.0;
    andorra_bb.lonShift = 0.0;
    andorra_bb.lowerLeft.lat = 39.644;
    andorra_bb.lowerLeft.lon = 1.414;
    andorra_bb.upperRight.lat = 41.248;
    andorra_bb.upperRight.lon = 1.787;
    retn.insert(QLatin1String("Andorra"), andorra_bb);

    BoundingBox armenia_bb;
    armenia_bb.latShift = 0.0;
    armenia_bb.lonShift = 0.0;
    armenia_bb.lowerLeft.lat = 38.741;
    armenia_bb.lowerLeft.lon = 43.583;
    armenia_bb.upperRight.lat = 41.248;
    armenia_bb.upperRight.lon = 46.505;
    retn.insert(QLatin1String("Armenia"), armenia_bb);

    BoundingBox austria_bb;
    austria_bb.latShift = 0.0;
    austria_bb.lonShift = 0.0;
    austria_bb.lowerLeft.lat = 46.432;
    austria_bb.lowerLeft.lon = 9.480;
    austria_bb.upperRight.lat = 49.039;
    austria_bb.upperRight.lon = 16.980;
    retn.insert(QLatin1String("Austria"), austria_bb);

    BoundingBox azerbaijan_bb;
    azerbaijan_bb.latShift = 0.0;
    azerbaijan_bb.lonShift = 0.0;
    azerbaijan_bb.lowerLeft.lat = 38.2704;
    azerbaijan_bb.lowerLeft.lon = 44.794;
    azerbaijan_bb.upperRight.lat = 41.861;
    azerbaijan_bb.upperRight.lon = 50.393;
    retn.insert(QLatin1String("Azerbaijan"), azerbaijan_bb);

    BoundingBox belarus_bb;
    belarus_bb.latShift = 0.0;
    belarus_bb.lonShift = 0.0;
    belarus_bb.lowerLeft.lat = 51.320;
    belarus_bb.lowerLeft.lon = 23.199;
    belarus_bb.upperRight.lat = 56.1691;
    belarus_bb.upperRight.lon = 32.694;
    retn.insert(QLatin1String("Belarus"), belarus_bb);

    BoundingBox belgium_bb;
    belgium_bb.latShift = 0.0;
    belgium_bb.lonShift = 0.0;
    belgium_bb.lowerLeft.lat = 49.529;
    belgium_bb.lowerLeft.lon = 2.514;
    belgium_bb.upperRight.lat = 51.475;
    belgium_bb.upperRight.lon = 6.157;
    retn.insert(QLatin1String("Belgium"), belgium_bb);

    BoundingBox bosnia_and_herzegovina_bb;
    bosnia_and_herzegovina_bb.latShift = 0.0;
    bosnia_and_herzegovina_bb.lonShift = 0.0;
    bosnia_and_herzegovina_bb.lowerLeft.lat = 42.650;
    bosnia_and_herzegovina_bb.lowerLeft.lon = 15.750;
    bosnia_and_herzegovina_bb.upperRight.lat = 45.234;
    bosnia_and_herzegovina_bb.upperRight.lon = 19.600;
    retn.insert(QLatin1String("Bosnia_and_Herzegovina"), bosnia_and_herzegovina_bb);

    BoundingBox bulgaria_bb;
    bulgaria_bb.latShift = 0.0;
    bulgaria_bb.lonShift = 0.0;
    bulgaria_bb.lowerLeft.lat = 41.234;
    bulgaria_bb.lowerLeft.lon = 22.381;
    bulgaria_bb.upperRight.lat = 44.235;
    bulgaria_bb.upperRight.lon = 28.558;
    retn.insert(QLatin1String("Bulgaria"), bulgaria_bb);

    BoundingBox croatia_bb;
    croatia_bb.latShift = 0.0;
    croatia_bb.lonShift = 0.0;
    croatia_bb.lowerLeft.lat = 42.490;
    croatia_bb.lowerLeft.lon = 13.657;
    croatia_bb.upperRight.lat = 46.504;
    croatia_bb.upperRight.lon = 19.390;
    retn.insert(QLatin1String("Croatia"), croatia_bb);

    BoundingBox cyprus_bb;
    cyprus_bb.latShift = 0.0;
    cyprus_bb.lonShift = 0.0;
    cyprus_bb.lowerLeft.lat = 34.572;
    cyprus_bb.lowerLeft.lon = 32.257;
    cyprus_bb.upperRight.lat = 35.173;
    cyprus_bb.upperRight.lon = 34.005;
    retn.insert(QLatin1String("Cyprus"), cyprus_bb);

    BoundingBox czech_republic_bb;
    czech_republic_bb.latShift = 0.0;
    czech_republic_bb.lonShift = 0.0;
    czech_republic_bb.lowerLeft.lat = 48.555;
    czech_republic_bb.lowerLeft.lon = 12.240;
    czech_republic_bb.upperRight.lat = 51.117;
    czech_republic_bb.upperRight.lon = 18.853;
    retn.insert(QLatin1String("Czech_Republic"), czech_republic_bb);

    BoundingBox denmark_bb;
    denmark_bb.latShift = 0.0;
    denmark_bb.lonShift = 0.0;
    denmark_bb.lowerLeft.lat = 54.800;
    denmark_bb.lowerLeft.lon = 8.090;
    denmark_bb.upperRight.lat = 57.7300;
    denmark_bb.upperRight.lon = 12.690;
    retn.insert(QLatin1String("Denmark"), denmark_bb);

    BoundingBox estonia_bb;
    estonia_bb.latShift = 0.0;
    estonia_bb.lonShift = 0.0;
    estonia_bb.lowerLeft.lat = 57.475;
    estonia_bb.lowerLeft.lon = 23.340;
    estonia_bb.upperRight.lat = 59.611;
    estonia_bb.upperRight.lon = 28.132;
    retn.insert(QLatin1String("Estonia"), estonia_bb);

    BoundingBox faroe_islands_bb;
    faroe_islands_bb.latShift = 0.0;
    faroe_islands_bb.lonShift = 0.0;
    faroe_islands_bb.lowerLeft.lat = 61.395;
    faroe_islands_bb.lowerLeft.lon = -7.681;
    faroe_islands_bb.upperRight.lat = 62.401;
    faroe_islands_bb.upperRight.lon = -6.259;
    retn.insert(QLatin1String("Faroe_Islands"), faroe_islands_bb);

    BoundingBox finland_bb;
    finland_bb.latShift = 0.0;
    finland_bb.lonShift = 0.0;
    finland_bb.lowerLeft.lat = 59.45;
    finland_bb.lowerLeft.lon = 18.0;
    finland_bb.upperRight.lat = 70.083;
    finland_bb.upperRight.lon = 33.35;
    retn.insert(QLatin1String("Finland"), finland_bb);

    BoundingBox france_bb;
    france_bb.latShift = 0.0;
    france_bb.lonShift = 0.0;
    france_bb.lowerLeft.lat = 2.0534;
    france_bb.lowerLeft.lon = -54.5248;
    france_bb.upperRight.lat = 51.149;
    france_bb.upperRight.lon = 9.560;
    retn.insert(QLatin1String("France"), france_bb);

    BoundingBox georgia_bb;
    georgia_bb.latShift = 0.0;
    georgia_bb.lonShift = 0.0;
    georgia_bb.lowerLeft.lat = 41.064;
    georgia_bb.lowerLeft.lon = 39.955;
    georgia_bb.upperRight.lat = 43.553;
    georgia_bb.upperRight.lon = 46.648;
    retn.insert(QLatin1String("Georgia"), georgia_bb);

    BoundingBox germany_bb;
    germany_bb.latShift = 0.0;
    germany_bb.lonShift = 0.0;
    germany_bb.lowerLeft.lat = 47.302;
    germany_bb.lowerLeft.lon = 5.989;
    germany_bb.upperRight.lat = 54.983;
    germany_bb.upperRight.lon = 15.017;
    retn.insert(QLatin1String("Germany"), germany_bb);

    BoundingBox gibraltar_bb;
    gibraltar_bb.latShift = 0.0;
    gibraltar_bb.lonShift = 0.0;
    gibraltar_bb.lowerLeft.lat = 36.108;
    gibraltar_bb.lowerLeft.lon = -5.358;
    gibraltar_bb.upperRight.lat = 36.156;
    gibraltar_bb.upperRight.lon = -5.339;
    retn.insert(QLatin1String("Gibraltar"), gibraltar_bb);

    BoundingBox greece_bb;
    greece_bb.latShift = 0.0;
    greece_bb.lonShift = 0.0;
    greece_bb.lowerLeft.lat = 34.920;
    greece_bb.lowerLeft.lon = 20.150;
    greece_bb.upperRight.lat = 41.827;
    greece_bb.upperRight.lon = 26.604;
    retn.insert(QLatin1String("Greece"), greece_bb);

    BoundingBox guernsey_bb;
    guernsey_bb.latShift = 0.0;
    guernsey_bb.lonShift = 0.0;
    guernsey_bb.lowerLeft.lat = 49.406;
    guernsey_bb.lowerLeft.lon = -2.675;
    guernsey_bb.upperRight.lat = 49.739;
    guernsey_bb.upperRight.lon = -2.164;
    retn.insert(QLatin1String("Guernsey"), guernsey_bb);

    BoundingBox hungary_bb;
    hungary_bb.latShift = 0.0;
    hungary_bb.lonShift = 0.0;
    hungary_bb.lowerLeft.lat = 45.759;
    hungary_bb.lowerLeft.lon = 16.202;
    hungary_bb.upperRight.lat = 48.624;
    hungary_bb.upperRight.lon = 22.711;
    retn.insert(QLatin1String("Hungary"), hungary_bb);

    BoundingBox iceland_bb;
    iceland_bb.latShift = 0.0;
    iceland_bb.lonShift = 0.0;
    iceland_bb.lowerLeft.lat = 63.496;
    iceland_bb.lowerLeft.lon = -24.326;
    iceland_bb.upperRight.lat = 66.526;
    iceland_bb.upperRight.lon = -13.609;
    retn.insert(QLatin1String("Iceland"), iceland_bb);

    BoundingBox ireland_bb;
    ireland_bb.latShift = 0.0;
    ireland_bb.lonShift = 0.0;
    ireland_bb.lowerLeft.lat = 51.669;
    ireland_bb.lowerLeft.lon = -9.977;
    ireland_bb.upperRight.lat = 55.132;
    ireland_bb.upperRight.lon = -6.030;
    retn.insert(QLatin1String("Ireland"), ireland_bb);

    BoundingBox isle_of_man_bb;
    isle_of_man_bb.latShift = 0.0;
    isle_of_man_bb.lonShift = 0.0;
    isle_of_man_bb.lowerLeft.lat = 54.045;
    isle_of_man_bb.lowerLeft.lon = -4.830;
    isle_of_man_bb.upperRight.lat = 54.419;
    isle_of_man_bb.upperRight.lon = -4.310;
    retn.insert(QLatin1String("Isle_of_Man"), isle_of_man_bb);

    BoundingBox italy_bb;
    italy_bb.latShift = 0.0;
    italy_bb.lonShift = 0.0;
    italy_bb.lowerLeft.lat = 36.620;
    italy_bb.lowerLeft.lon = 6.745;
    italy_bb.upperRight.lat = 47.115;
    italy_bb.upperRight.lon = 18.480;
    retn.insert(QLatin1String("Italy"), italy_bb);

    BoundingBox jersey_bb;
    jersey_bb.latShift = 0.0;
    jersey_bb.lonShift = 0.0;
    jersey_bb.lowerLeft.lat = 49.162;
    jersey_bb.lowerLeft.lon = -2.255;
    jersey_bb.upperRight.lat = 49.262;
    jersey_bb.upperRight.lon = -2.011;
    retn.insert(QLatin1String("Jersey"), jersey_bb);

    BoundingBox kazakhstan_bb;
    kazakhstan_bb.latShift = 0.0;
    kazakhstan_bb.lonShift = 0.0;
    kazakhstan_bb.lowerLeft.lat = 40.663;
    kazakhstan_bb.lowerLeft.lon = 46.466;
    kazakhstan_bb.upperRight.lat = 55.385;
    kazakhstan_bb.upperRight.lon = 87.360;
    retn.insert(QLatin1String("Kazakhstan"), kazakhstan_bb);

    BoundingBox kosovo_bb;
    kosovo_bb.latShift = 0.0;
    kosovo_bb.lonShift = 0.0;
    kosovo_bb.lowerLeft.lat = 42.111;
    kosovo_bb.lowerLeft.lon = 20.787;
    kosovo_bb.upperRight.lat = 43.153;
    kosovo_bb.upperRight.lon = 21.514;
    retn.insert(QLatin1String("Kosovo"), kosovo_bb);

    BoundingBox latvia_bb;
    latvia_bb.latShift = 0.0;
    latvia_bb.lonShift = 0.0;
    latvia_bb.lowerLeft.lat = 55.615;
    latvia_bb.lowerLeft.lon = 21.056;
    latvia_bb.upperRight.lat = 57.970;
    latvia_bb.upperRight.lon = 28.176;
    retn.insert(QLatin1String("Latvia"), latvia_bb);

    BoundingBox liechtenstein_bb;
    liechtenstein_bb.latShift = 0.0;
    liechtenstein_bb.lonShift = 0.0;
    liechtenstein_bb.lowerLeft.lat = 47.048;
    liechtenstein_bb.lowerLeft.lon = 9.472;
    liechtenstein_bb.upperRight.lat = 47.270;
    liechtenstein_bb.upperRight.lon = 9.636;
    retn.insert(QLatin1String("Liechtenstein"), liechtenstein_bb);

    BoundingBox lithuania_bb;
    lithuania_bb.latShift = 0.0;
    lithuania_bb.lonShift = 0.0;
    lithuania_bb.lowerLeft.lat = 53.906;
    lithuania_bb.lowerLeft.lon = 21.056;
    lithuania_bb.upperRight.lat = 56.373;
    lithuania_bb.upperRight.lon = 26.588;
    retn.insert(QLatin1String("Lithuania"), lithuania_bb);

    BoundingBox luxembourg_bb;
    luxembourg_bb.latShift = 0.0;
    luxembourg_bb.lonShift = 0.0;
    luxembourg_bb.lowerLeft.lat = 49.443;
    luxembourg_bb.lowerLeft.lon = 5.674;
    luxembourg_bb.upperRight.lat = 50.128;
    luxembourg_bb.upperRight.lon = 6.243;
    retn.insert(QLatin1String("Luxembourg"), luxembourg_bb);

    BoundingBox macedonia_bb;
    macedonia_bb.latShift = 0.0;
    macedonia_bb.lonShift = 0.0;
    macedonia_bb.lowerLeft.lat = 40.843;
    macedonia_bb.lowerLeft.lon = 20.463;
    macedonia_bb.upperRight.lat = 42.320;
    macedonia_bb.upperRight.lon = 22.952;
    retn.insert(QLatin1String("Macedonia"), macedonia_bb);

    BoundingBox malta_bb;
    malta_bb.latShift = 0.0;
    malta_bb.lonShift = 0.0;
    malta_bb.lowerLeft.lat = 35.786;
    malta_bb.lowerLeft.lon = 14.183;
    malta_bb.upperRight.lat = 36.082;
    malta_bb.upperRight.lon = 14.577;
    retn.insert(QLatin1String("Malta"), malta_bb);

    BoundingBox moldova_bb;
    moldova_bb.latShift = 0.0;
    moldova_bb.lonShift = 0.0;
    moldova_bb.lowerLeft.lat = 45.488;
    moldova_bb.lowerLeft.lon = 26.619;
    moldova_bb.upperRight.lat = 48.497;
    moldova_bb.upperRight.lon = 30.025;
    retn.insert(QLatin1String("Moldova"), moldova_bb);

    BoundingBox monaco_bb;
    monaco_bb.latShift = 0.0;
    monaco_bb.lonShift = 0.0;
    monaco_bb.lowerLeft.lat = 43.725;
    monaco_bb.lowerLeft.lon = 7.409;
    monaco_bb.upperRight.lat = 43.752;
    monaco_bb.upperRight.lon = 7.439;
    retn.insert(QLatin1String("Monaco"), monaco_bb);

    BoundingBox montenegro_bb;
    montenegro_bb.latShift = 0.0;
    montenegro_bb.lonShift = 0.0;
    montenegro_bb.lowerLeft.lat = 41.878;
    montenegro_bb.lowerLeft.lon = 18.45;
    montenegro_bb.upperRight.lat = 43.524;
    montenegro_bb.upperRight.lon = 20.340;
    retn.insert(QLatin1String("Montenegro"), montenegro_bb);

    BoundingBox netherlands_bb;
    netherlands_bb.latShift = 0.0;
    netherlands_bb.lonShift = 0.0;
    netherlands_bb.lowerLeft.lat = 50.804;
    netherlands_bb.lowerLeft.lon = 3.315;
    netherlands_bb.upperRight.lat = 53.510;
    netherlands_bb.upperRight.lon = 7.092;
    retn.insert(QLatin1String("Netherlands"), netherlands_bb);

    BoundingBox norway_bb;
    norway_bb.latShift = 0.0;
    norway_bb.lonShift = 0.0;
    norway_bb.lowerLeft.lat = 58.079;
    norway_bb.lowerLeft.lon = 4.992;
    norway_bb.upperRight.lat = 80.657;
    norway_bb.upperRight.lon = 31.293;
    retn.insert(QLatin1String("Norway"), norway_bb);

    BoundingBox poland_bb;
    poland_bb.latShift = 0.0;
    poland_bb.lonShift = 0.0;
    poland_bb.lowerLeft.lat = 49.027;
    poland_bb.lowerLeft.lon = 14.075;
    poland_bb.upperRight.lat = 54.851;
    poland_bb.upperRight.lon = 24.030;
    retn.insert(QLatin1String("Poland"), poland_bb);

    BoundingBox portugal_bb;
    portugal_bb.latShift = 0.0;
    portugal_bb.lonShift = 0.0;
    portugal_bb.lowerLeft.lat = 36.838;
    portugal_bb.lowerLeft.lon = -9.527;
    portugal_bb.upperRight.lat = 42.280;
    portugal_bb.upperRight.lon = -6.389;
    retn.insert(QLatin1String("Portugal"), portugal_bb);

    BoundingBox romania_bb;
    romania_bb.latShift = 0.0;
    romania_bb.lonShift = 0.0;
    romania_bb.lowerLeft.lat = 43.688;
    romania_bb.lowerLeft.lon = 20.220;
    romania_bb.upperRight.lat = 48.221;
    romania_bb.upperRight.lon = 29.637;
    retn.insert(QLatin1String("Romania"), romania_bb);

    BoundingBox san_marino_bb;
    san_marino_bb.latShift = 0.0;
    san_marino_bb.lonShift = 0.0;
    san_marino_bb.lowerLeft.lat = 43.893;
    san_marino_bb.lowerLeft.lon = 12.403;
    san_marino_bb.upperRight.lat = 43.992;
    san_marino_bb.upperRight.lon = 12.517;
    retn.insert(QLatin1String("San_Marino"), san_marino_bb);

    BoundingBox serbia_bb;
    serbia_bb.latShift = 0.0;
    serbia_bb.lonShift = 0.0;
    serbia_bb.lowerLeft.lat = 42.245;
    serbia_bb.lowerLeft.lon = 18.830;
    serbia_bb.upperRight.lat = 46.171;
    serbia_bb.upperRight.lon = 22.986;
    retn.insert(QLatin1String("Serbia"), serbia_bb);

    BoundingBox slovakia_bb;
    slovakia_bb.latShift = 0.0;
    slovakia_bb.lonShift = 0.0;
    slovakia_bb.lowerLeft.lat = 47.758;
    slovakia_bb.lowerLeft.lon = 16.880;
    slovakia_bb.upperRight.lat = 49.571;
    slovakia_bb.upperRight.lon = 22.558;
    retn.insert(QLatin1String("Slovakia"), slovakia_bb);

    BoundingBox slovenia_bb;
    slovenia_bb.latShift = 0.0;
    slovenia_bb.lonShift = 0.0;
    slovenia_bb.lowerLeft.lat = 45.452;
    slovenia_bb.lowerLeft.lon = 13.698;
    slovenia_bb.upperRight.lat = 46.852;
    slovenia_bb.upperRight.lon = 16.565;
    retn.insert(QLatin1String("Slovenia"), slovenia_bb);

    BoundingBox spain_bb;
    spain_bb.latShift = 0.0;
    spain_bb.lonShift = 0.0;
    spain_bb.lowerLeft.lat = 35.947;
    spain_bb.lowerLeft.lon = -9.393;
    spain_bb.upperRight.lat = 43.748;
    spain_bb.upperRight.lon = 3.039;
    retn.insert(QLatin1String("Spain"), spain_bb);

    BoundingBox sweden_bb;
    sweden_bb.latShift = 0.0;
    sweden_bb.lonShift = 0.0;
    sweden_bb.lowerLeft.lat = 55.362;
    sweden_bb.lowerLeft.lon = 11.027;
    sweden_bb.upperRight.lat = 69.106;
    sweden_bb.upperRight.lon = 23.903;
    retn.insert(QLatin1String("Sweden"), sweden_bb);

    BoundingBox switzerland_bb;
    switzerland_bb.latShift = 0.0;
    switzerland_bb.lonShift = 0.0;
    switzerland_bb.lowerLeft.lat = 45.777;
    switzerland_bb.lowerLeft.lon = 6.023;
    switzerland_bb.upperRight.lat = 47.831;
    switzerland_bb.upperRight.lon = 10.443;
    retn.insert(QLatin1String("Switzerland"), switzerland_bb);

    BoundingBox ukraine_bb;
    ukraine_bb.latShift = 0.0;
    ukraine_bb.lonShift = 0.0;
    ukraine_bb.lowerLeft.lat = 44.361;
    ukraine_bb.lowerLeft.lon = 22.057;
    ukraine_bb.upperRight.lat = 52.335;
    ukraine_bb.upperRight.lon = 40.081;
    retn.insert(QLatin1String("Ukraine"), ukraine_bb);

    BoundingBox united_kingdom_bb;
    united_kingdom_bb.latShift = 0.0;
    united_kingdom_bb.lonShift = 0.0;
    united_kingdom_bb.lowerLeft.lat = 49.960;
    united_kingdom_bb.lowerLeft.lon = -7.572;
    united_kingdom_bb.upperRight.lat = 58.635;
    united_kingdom_bb.upperRight.lon = 1.681;
    retn.insert(QLatin1String("United_Kingdom"), united_kingdom_bb);

    BoundingBox india_bb;
    india_bb.latShift = 0.0;
    india_bb.lonShift = 0.0;
    india_bb.lowerLeft.lat = 6.0;
    india_bb.lowerLeft.lon = 65.0;
    india_bb.upperRight.lat = 35.956;
    india_bb.upperRight.lon = 97.35;
    retn.insert(QLatin1String("India"), india_bb);

    BoundingBox australia_bb;
    australia_bb.latShift = 0.0;
    australia_bb.lonShift = 0.0;
    australia_bb.lowerLeft.lat = -55.05;
    australia_bb.lowerLeft.lon = 112.467;
    australia_bb.upperRight.lat = -9.133;
    australia_bb.upperRight.lon = 168.0;
    retn.insert(QLatin1String("Australia"), australia_bb);

    return retn;
}

QMap<QString, QVector<BoundingBox> > regionBoundingBoxes()
{
    QMap<QString, QVector<BoundingBox> > retn;
    QMap<QString, BoundingBox> cbboxes(countryBoundingBoxes());

    // Devel is a special region for development/testing purposes.
    // Devel is not able to be represented by a single bounding box.
    // Instead, we hardcode its contents to include the bounding boxes
    // for Finland, Australia and India.
    QVector<BoundingBox> devel;
    devel << cbboxes.value(QLatin1String("Finland"))
          << cbboxes.value(QLatin1String("Australia"))
          << cbboxes.value(QLatin1String("India"));
    retn.insert(QLatin1String("Devel"), devel);

    return retn;
}

struct ParseLineResult {
    bool withinBBox;
    MlsdbUniqueCellId uniqueCellId;
    MlsdbCoords loc;
};
ParseLineResult parseLineAndTest(const QByteArray &line, const QVector<BoundingBox> &boundingBoxes)
{
    ParseLineResult result;
    result.withinBBox = false;

    // radio,mcc,net,area,cell,unit,lon,lat,range,samples,changeable,created,updated,averageSignal
    QList<QByteArray> fields = line.split(',');
    // grab the type of cell (LTE, GSM, UMTS, etc)
    QString cellTypeStr = QString::fromLatin1(fields[0]);
    MlsdbCellType cellType = MLSDB_CELL_TYPE_LTE;
    if (cellTypeStr.compare(QLatin1String("LTE"), Qt::CaseInsensitive) == 0) {
        cellType = MLSDB_CELL_TYPE_LTE;
    } else if (cellTypeStr.compare(QLatin1String("GSM"), Qt::CaseInsensitive) == 0) {
        cellType = MLSDB_CELL_TYPE_GSM;
    } else if (cellTypeStr.compare(QLatin1String("UMTS"), Qt::CaseInsensitive) == 0) {
        cellType = MLSDB_CELL_TYPE_UMTS;
    } else {
        cellType = MLSDB_CELL_TYPE_OTHER;
    }
    // parse mcc and mnc fields
    quint16 mcc = QString::fromLatin1(fields[1]).isEmpty() ? 0 : QString::fromLatin1(fields[1]).toUInt();
    quint16 mnc = QString::fromLatin1(fields[2]).isEmpty() ? 0 : QString::fromLatin1(fields[2]).toUInt();
    // check to see that there is a cellId associated.  not the case for some UMTS cells.
    quint32 cellId = QString::fromLatin1(fields[4]).isEmpty() ? 0 : QString::fromLatin1(fields[4]).toUInt();
    quint32 locationCode = QString::fromLatin1(fields[3]).isEmpty() ? 0 : QString::fromLatin1(fields[3]).toUInt();
    if (cellId > 0) {
        // build the cell location.
        result.uniqueCellId = MlsdbUniqueCellId(cellType, cellId, locationCode, mcc, mnc);
        result.loc.lat = QString::fromLatin1(fields[7]).toDouble();
        result.loc.lon = QString::fromLatin1(fields[6]).toDouble();
        // check to see if it's within our bounding boxes
        Q_FOREACH (const BoundingBox &bbox, boundingBoxes) {
            if (bbox.contains(result.loc)) {
                result.withinBBox = true;
                return result;
            }
        }
    }
    return result;
}

QMap<MlsdbUniqueCellId, MlsdbCoords> siftMlsCsvData(const QString &mlscsv, const QVector<BoundingBox> &boundingBoxes)
{
    QMap<MlsdbUniqueCellId, MlsdbCoords> cellIdToMlsdbCoords;

    // read the csv file line by line, check whether it belongs in the map, insert or discard.
    QFile file(mlscsv);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fprintf(stderr, "Failed to open Mozilla Location Services data .csv file for reading!\n");
        return cellIdToMlsdbCoords;
    }

    fprintf(stdout, "Reading data from file and caching cell locations which are within the bounding boxes...\n");
    quint32 readlines = 0, insertedcells = 0, progresscount = 0;
#if USE_CONCURRENT
    int currentJobs = THREADCOUNT-1;
#if USE_STD_CONCURRENT
    std::vector<std::future<ParseLineResult> > results;
#else
    QList<QFuture<ParseLineResult> > results;
#endif // USE_STD_CONCURRENT
#endif // USE_CONCURRENT
    fprintf(stdout, "Progress: %d lines read, %d cell locations inserted", readlines, insertedcells);
    while (true) {
        if (file.atEnd()) break;
#if USE_CONCURRENT
        currentJobs = THREADCOUNT-1;
        results.clear();
        while (!file.atEnd() && currentJobs) {
            currentJobs--;
            const QByteArray line = file.readLine();
            readlines++;
#if USE_STD_CONCURRENT
            results.push_back(std::async(parseLineAndTest, line, boundingBoxes));
#else
            results.append(QtConcurrent::run(parseLineAndTest, line, boundingBoxes));
#endif // USE_STD_CONCURRENT
        }

#if USE_STD_CONCURRENT
        for (auto &result : results) {
            ParseLineResult r = result.get();
#else
        while (results.size()) {
            result.waitForFinished();
            ParseLineResult r = result.result();
#endif // USE_STD_CONCURRENT
#else  // single-threaded only
        {
            QByteArray line = file.readLine();
            readlines++;
            ParseLineResult r = parseLineAndTest(line, boundingBoxes);
#endif
            if (r.withinBBox) {
                insertedcells++;
                cellIdToMlsdbCoords.insert(r.uniqueCellId, r.loc);
            }
            progresscount++;
        }

        if (progresscount >= 10000) {
            progresscount = 0;
            fprintf(stdout, "\33[2K\rProgress: %d lines read, %d cell locations inserted", readlines, insertedcells); // line overwrite
            fflush(stdout);
        }
    }

    fprintf(stdout, "\nFinished reading data: %d lines read, %d cell locations inserted\n", readlines, insertedcells);
    return cellIdToMlsdbCoords;
}

int writeMlsdbData(const QMap<MlsdbUniqueCellId, MlsdbCoords> &cellIdToLocation)
{
    if (cellIdToLocation.isEmpty()) {
        fprintf(stderr, "No cell locations found which match the required bounding boxes!\n");
        return 1;
    }

    // split the complete cellIdToLocation dictionary into per-location-code-first-digit sub-dictionaries
    QMap<QChar, QMap<MlsdbUniqueCellId, MlsdbCoords> > perLcfdCellIdToLocation;
    for (QMap<MlsdbUniqueCellId, MlsdbCoords>::const_iterator it = cellIdToLocation.constBegin();
         it != cellIdToLocation.constEnd(); it++) {
        perLcfdCellIdToLocation[QString::number(it.key().locationCode()).at(0)].insert(it.key(), it.value());
    }

    // write each sub-dictionary into a first-digit-of-location-code-specific directory.
    QDir dir;
    for (QMap<QChar, QMap<MlsdbUniqueCellId, MlsdbCoords> >::const_iterator it = perLcfdCellIdToLocation.constBegin();
         it != perLcfdCellIdToLocation.constEnd(); ++it) {
        // create the directory
        dir.mkpath(QStringLiteral("./%1").arg(it.key()));
        // write the data file
        const QString fileName(QStringLiteral("./%1/mlsdb.data").arg(it.key()));
        fprintf(stdout, "Writing data (%d entries) to mlsdb.data file: %s\n", it.value().size(), fileName.toStdString().c_str());
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        QDataStream out(&file);
        out << (quint32)0xc710cdb; // magic cell-tower location db number
        out << (qint32)3;          // data file version number
        out.setVersion(QDataStream::Qt_5_0);
        out << it.value();
    }

    fprintf(stdout, "Done!\n");
    return 0; // success
}

int queryCellLocation(quint32 locationAreaCode, quint32 cellId, quint16 mcc, quint16 mnc, const QString &mlsdbdata)
{
    if (cellId == 0) {
        fprintf(stderr, "Invalid cellId specified!\n");
        return -1;
    }

    if (!QFile::exists(mlsdbdata)) {
        fprintf(stderr, "geoclue-mlsdb data file does not exist: %s\n", mlsdbdata.toStdString().c_str());
        return 1;
    }

    fprintf(stdout, "Reading data from mlsdb.data file...\n");
    QFile file(mlsdbdata);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    quint32 magic = 0, expectedMagic = (quint32)0xc710cdb;
    in >> magic;
    if (magic != 0xc710cdb) {
        fprintf(stderr, "geoclue-mlsdb data file format unknown: %d (expect %d)\n", magic, expectedMagic);
        return 1;
    }
    qint32 version;
    in >> version;
    if (version != 3) {
        fprintf(stderr, "geoclue-mlsdb data file version unknown: %d\n", version);
        return 1;
    }
    QMap<MlsdbUniqueCellId, MlsdbCoords> uniqueCellIdToLocation;
    in >> uniqueCellIdToLocation;
    if (uniqueCellIdToLocation.isEmpty()) {
        fprintf(stderr, "geoclue-mlsdb data file contained no cell locations!\n");
        return 1;
    }

    fprintf(stdout, "Searching for %d within data...\n", cellId);
    MlsdbUniqueCellId lteCellId = MlsdbUniqueCellId(MLSDB_CELL_TYPE_LTE, cellId, locationAreaCode, mcc, mnc);
    MlsdbUniqueCellId gsmCellId = MlsdbUniqueCellId(MLSDB_CELL_TYPE_GSM, cellId, locationAreaCode, mcc, mnc);
    MlsdbUniqueCellId umtsCellId = MlsdbUniqueCellId(MLSDB_CELL_TYPE_UMTS, cellId, locationAreaCode, mcc, mnc);
    if (!uniqueCellIdToLocation.contains(lteCellId) && !uniqueCellIdToLocation.contains(gsmCellId) && !uniqueCellIdToLocation.contains(umtsCellId)) {
        fprintf(stderr, "geoclue-mlsdb data file does not contain location of cell with id: %d in LAC: %d with MCC: %d, MNC: %d\n", cellId, locationAreaCode, mcc, mnc);
        return 1;
    }

    if (uniqueCellIdToLocation.contains(lteCellId)) {
        MlsdbCoords cellLocation = uniqueCellIdToLocation.value(lteCellId);
        fprintf(stdout, "LTE cell with id %d is at lat,lon: %f, %f\n", lteCellId.cellId(), cellLocation.lat, cellLocation.lon);
    }

    if (uniqueCellIdToLocation.contains(gsmCellId)) {
        MlsdbCoords cellLocation = uniqueCellIdToLocation.value(gsmCellId);
        fprintf(stdout, "GSM cell with id %d is at lat,lon: %f, %f\n", gsmCellId.cellId(), cellLocation.lat, cellLocation.lon);
    }

    if (uniqueCellIdToLocation.contains(umtsCellId)) {
        MlsdbCoords cellLocation = uniqueCellIdToLocation.value(umtsCellId);
        fprintf(stdout, "UMTS cell with id %d is at lat,lon: %f, %f\n", umtsCellId.cellId(), cellLocation.lat, cellLocation.lon);
    }

    return 0; // success
}

int generateCountryDb(const QString &country, const QString &mlscsv)
{
    if (!QFile::exists(mlscsv)) {
        fprintf(stderr, "Mozilla Location Services .csv data file does not exist: %s\n", mlscsv.toStdString().c_str());
        return 1;
    }

    QMap<QString, BoundingBox> cbb = countryBoundingBoxes();
    if (!cbb.contains(country)) {
        fprintf(stderr, "Country bounding box not known: %s.  Note: case sensitive!\n", country.toStdString().c_str());
        return 1;
    }

    return writeMlsdbData(siftMlsCsvData(mlscsv, QVector<BoundingBox>() << cbb.value(country)));
}

int generateRegionDb(const QString &region, const QString &mlscsv)
{
    if (!QFile::exists(mlscsv)) {
        fprintf(stderr, "Mozilla Location Services .csv data file does not exist: %s\n", mlscsv.toStdString().c_str());
        return 1;
    }

    QMap<QString, QVector<BoundingBox> > rbb = regionBoundingBoxes();
    if (!rbb.contains(region)) {
        fprintf(stderr, "Region bounding box not known: %s.  Note: case sensitive!\n", region.toStdString().c_str());
        return 1;
    }

    return writeMlsdbData(siftMlsCsvData(mlscsv, rbb.value(region)));
}

void printGenericHelp()
{
    fprintf(stdout, "geoclue-mlsdb-tool is used to generate a datastructure containing"
                    " country- or region-specific GSM/UMTS/LTE cell location information.\n");
    fprintf(stdout, "Usage:\n\tgeoclue-mlsdb-tool --help [country|region]\n"
                    "\tgeoclue-mlsdb-tool -c <country> mls.csv\n"
                    "\tgeoclue-mlsdb-tool -r <region> mls.csv\n"
                    "\tgeoclue-mlsdb-tool --query <locationId> <cellId> <mcc> <mnc> mlsdb.data\n");
    fprintf(stdout, "For more information about valid country and region arguments,"
                    " run `geoclue-mlsdb-tool --help country` or `geoclue-mlsdb-tool --help region`.\n");
}

void printCountryHelp()
{
    fprintf(stdout, "Running `geoclue-mlsdb-tool -c <country> mls.csv` will generate an output"
                    " file `mlsdb.data` which contains a mapping from (uint32) cell id"
                    " to (double,double) latitude,longitude for all cells known"
                    " in the mls.csv within the bounding-box for that country.\n");
    fprintf(stdout, "Valid countries are:\n");
    fprintf(stdout, "\tNAME            \tLowerLeft(lat,lon)\tUpperRight(lat,lon)\n");
    const QMap<QString, BoundingBox> cbb(countryBoundingBoxes());
    Q_FOREACH (const QString &c, cbb.keys()) {
        const BoundingBox &bbox(cbb[c]);
        if (bbox.latShift == 0.0 && bbox.lonShift == 0.0) {
            fprintf(stdout, "\t%16s\t%f,%f\t%f,%f\n",
                    c.toStdString().c_str(),
                    bbox.lowerLeft.lat, bbox.lowerLeft.lon,
                    bbox.upperRight.lat, bbox.upperRight.lon);
        } else {
            fprintf(stdout, "\t%16s\t(complex bbox - crosses meridian or pole)\n",
                    c.toStdString().c_str());
        }
    }
}

void printRegionHelp()
{
    fprintf(stdout, "Running `geoclue-mlsdb-tool -r <region> mls.csv` will generate an output"
                    " file `mlsdb.data` which contains a mapping from (uint32) cell id"
                    " to (double,double) latitude,longitude for all cells known"
                    " in the mls.csv within the bounding-box for that region.\n"
                    "One special region is the `Devel` region which contains just"
                    " Finland, Australia and India.\n");
    fprintf(stdout, "Valid regions are:\n");
    const QMap<QString, QVector<BoundingBox> > rbb(regionBoundingBoxes());
    Q_FOREACH (const QString &r, rbb.keys()) {
        fprintf(stdout, "\t%s\n", r.toStdString().c_str());
    }
}

void printUsageError()
{
    fprintf(stderr, "usage:\n"
                    "\tgeoclue-mlsdb-tool --help [country|region]\n"
                    "\tgeoclue-mlsdb-tool -c <country> mls.csv\n"
                    "\tgeoclue-mlsdb-tool -r <region> mls.csv\n"
                    "\tgeoclue-mlsdb-tool --query <locationId> <cellId> <mcc> <mnc> mlsdb.data\n");
}

void detectDuplicates(const QString &mlscsv)
{
    QMultiMap<MlsdbUniqueCellId, MlsdbCoords> coords;
    const BoundingBox &bbox(countryBoundingBoxes().value(QStringLiteral("India")));

    // read the csv file line by line, check whether it belongs in the map, insert or discard.
    QFile file(mlscsv);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fprintf(stderr, "Failed to open Mozilla Location Services data .csv file for reading!\n");
        return;
    }
    quint32 readlines = 0, progresscount = 0, insertedcells = 0;
    while (true) {
        if (file.atEnd()) break;
        QByteArray line = file.readLine();
        readlines++;
        ParseLineResult r = parseLineAndTest(line, QVector<BoundingBox>() << bbox);
        if (r.withinBBox) {
            insertedcells++;
            coords.insert(r.uniqueCellId, r.loc);
        }
        progresscount++;
        if (progresscount >= 10000) {
            progresscount = 0;
            fprintf(stdout, "\33[2K\rProgress: %d lines read, %d cell locations inserted", readlines, insertedcells); // line overwrite
            fflush(stdout);
        }
    }

    fprintf(stdout, "\nFinished: %d lines read, %d cell locations inserted\n", readlines, insertedcells);

    // determine if the locations are different
    Q_FOREACH (const MlsdbUniqueCellId &cellId, coords.keys()) {
        QList<MlsdbCoords> locs = coords.values(cellId);
        if (locs.size() > 1) {
            Q_FOREACH (const MlsdbCoords &loc, locs) {
                fprintf(stdout, "Have duplicate: type: %s, cellId: %d, locCode: %d, mcc: %d, mnc: %d, lat: %f, lon: %f\n",
                        stringForMlsdbCellType(cellId.cellType()).toStdString().c_str(),
                        cellId.cellId(), cellId.locationCode(), cellId.mcc(), cellId.mnc(),
                        loc.lat, loc.lon);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();

    if (args.length() <= 1) {
        printGenericHelp();
        return 0;
    }

    if (args[1] == QStringLiteral("help")
            || args[1] == QStringLiteral("-help")
            || args[1] == QStringLiteral("--help")
            || args[1] == QStringLiteral("-h")
            || args[1] == QStringLiteral("--h")) {
        if (args.length() == 2) {
            printGenericHelp();
        } else if (args.length() > 3) {
            printUsageError();
            return 1;
        } else {
            if (args[2] == QStringLiteral("country")) {
                printCountryHelp();
            } else if (args[2] == QStringLiteral("region")) {
                printRegionHelp();
            }
        }
        return 0;
    }

    bool countryRequest = args[1].compare(QStringLiteral("-c"), Qt::CaseInsensitive) == 0;
    bool regionRequest  = args[1].compare(QStringLiteral("-r"), Qt::CaseInsensitive) == 0;
    if (args.length() == 4 && (countryRequest || regionRequest)) {
        return countryRequest ? generateCountryDb(args[2], args[3]) : generateRegionDb(args[2], args[3]);
    } else if (args.length() == 7 && args[1].compare(QStringLiteral("--query"), Qt::CaseInsensitive) == 0) {
        return queryCellLocation(args[2].toUInt(), args[3].toUInt(), args[4].toUInt(), args[5].toUInt(), args[6]);
    }

    printUsageError();
    return 1;
}
