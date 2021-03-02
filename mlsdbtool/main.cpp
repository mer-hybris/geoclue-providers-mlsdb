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
    double latShift = 0.0;
    double lonShift = 0.0;

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

// Africa

    BoundingBox algeria_bb;
    algeria_bb.lowerLeft.lat = 19.057;
    algeria_bb.lowerLeft.lon = -8.684;
    algeria_bb.upperRight.lat = 37.118;
    algeria_bb.upperRight.lon = 11.999;
    retn.insert(QLatin1String("Algeria"), algeria_bb);

    BoundingBox angola_bb;
    angola_bb.lowerLeft.lat = -17.931;
    angola_bb.lowerLeft.lon = 11.6400;
    angola_bb.upperRight.lat = -4.438;
    angola_bb.upperRight.lon = 24.080;
    retn.insert(QLatin1String("Angola"), angola_bb);

    BoundingBox benin_bb;
    benin_bb.lowerLeft.lat = 6.142;
    benin_bb.lowerLeft.lon = 0.772;
    benin_bb.upperRight.lat = 12.236;
    benin_bb.upperRight.lon = 3.797;
    retn.insert(QLatin1String("Benin"), benin_bb);

    BoundingBox botswana_bb;
    botswana_bb.lowerLeft.lat = -26.828;
    botswana_bb.lowerLeft.lon = 19.895;
    botswana_bb.upperRight.lat = -17.661;
    botswana_bb.upperRight.lon = 29.432;
    retn.insert(QLatin1String("Botswana"), botswana_bb);

    BoundingBox burkina_faso_bb;
    burkina_faso_bb.lowerLeft.lat = 9.611;
    burkina_faso_bb.lowerLeft.lon = -5.471;
    burkina_faso_bb.upperRight.lat = 15.116;
    burkina_faso_bb.upperRight.lon = 2.177;
    retn.insert(QLatin1String("Burkina_Faso"), burkina_faso_bb);

    BoundingBox burundi_bb;
    burundi_bb.lowerLeft.lat = -4.499;
    burundi_bb.lowerLeft.lon = 29.025;
    burundi_bb.upperRight.lat = -2.348;
    burundi_bb.upperRight.lon = 30.752;
    retn.insert(QLatin1String("Burundi"), burundi_bb);

    BoundingBox cameroon_bb;
    cameroon_bb.lowerLeft.lat = 1.728;
    cameroon_bb.lowerLeft.lon = 8.488;
    cameroon_bb.upperRight.lat = 12.859;
    cameroon_bb.upperRight.lon = 16.013;
    retn.insert(QLatin1String("Cameroon"), cameroon_bb);

    BoundingBox canary_islands_bb;
    canary_islands_bb.lowerLeft.lat = 27.637;
    canary_islands_bb.lowerLeft.lon = -18.161;
    canary_islands_bb.upperRight.lat = 29.415;
    canary_islands_bb.upperRight.lon = -13.419;
    retn.insert(QLatin1String("Canary_Islands"), canary_islands_bb);

    BoundingBox cape_verde_bb;
    cape_verde_bb.lowerLeft.lat = 14.802;
    cape_verde_bb.lowerLeft.lon = -25.359;
    cape_verde_bb.upperRight.lat = 17.197;
    cape_verde_bb.upperRight.lon = -22.666;
    retn.insert(QLatin1String("Cape_Verde"), cape_verde_bb);

    BoundingBox central_african_republic_bb;
    central_african_republic_bb.lowerLeft.lat = 2.267;
    central_african_republic_bb.lowerLeft.lon = 14.459;
    central_african_republic_bb.upperRight.lat = 11.142;
    central_african_republic_bb.upperRight.lon = 27.374;
    retn.insert(QLatin1String("Central_African_Republic"), central_african_republic_bb);

    BoundingBox ceuta_bb;
    ceuta_bb.lowerLeft.lat = 35.871;
    ceuta_bb.lowerLeft.lon = -5.353;
    ceuta_bb.upperRight.lat = 35.907;
    ceuta_bb.upperRight.lon = -5.278;
    retn.insert(QLatin1String("Ceuta"), ceuta_bb);

    BoundingBox chad_bb;
    chad_bb.lowerLeft.lat = 7.422;
    chad_bb.lowerLeft.lon = 13.540;
    chad_bb.upperRight.lat = 23.410;
    chad_bb.upperRight.lon = 23.889;
    retn.insert(QLatin1String("Chad"), chad_bb);

    BoundingBox comoros_bb;
    comoros_bb.lowerLeft.lat = -12.414;
    comoros_bb.lowerLeft.lon = 43.216;
    comoros_bb.upperRight.lat = -11.362;
    comoros_bb.upperRight.lon = 44.538;
    retn.insert(QLatin1String("Comoros"), comoros_bb);

    BoundingBox congo_bb;
    congo_bb.lowerLeft.lat = -5.038;
    congo_bb.lowerLeft.lon = 11.093;
    congo_bb.upperRight.lat = 3.728;
    congo_bb.upperRight.lon = 18.453;
    retn.insert(QLatin1String("Congo"), congo_bb);

    BoundingBox democratic_republic_of_the_congo_bb;
    democratic_republic_of_the_congo_bb.lowerLeft.lat = -13.257;
    democratic_republic_of_the_congo_bb.lowerLeft.lon = 12.182;
    democratic_republic_of_the_congo_bb.upperRight.lat = 5.256;
    democratic_republic_of_the_congo_bb.upperRight.lon = 31.174;
    retn.insert(QLatin1String("Democratic_Republic_of_the_Congo"), democratic_republic_of_the_congo_bb);

    BoundingBox djibouti_bb;
    djibouti_bb.lowerLeft.lat = 10.926;
    djibouti_bb.lowerLeft.lon = 41.662;
    djibouti_bb.upperRight.lat = 12.699;
    djibouti_bb.upperRight.lon = 43.318;
    retn.insert(QLatin1String("Djibouti"), djibouti_bb);

    BoundingBox egypt_bb;
    egypt_bb.lowerLeft.lat = 22.0;
    egypt_bb.lowerLeft.lon = 24.700;
    egypt_bb.upperRight.lat = 31.586;
    egypt_bb.upperRight.lon = 36.866;
    retn.insert(QLatin1String("Egypt"), egypt_bb);

    BoundingBox equatorial_guinea_bb;
    equatorial_guinea_bb.lowerLeft.lat = 1.010;
    equatorial_guinea_bb.lowerLeft.lon = 9.306;
    equatorial_guinea_bb.upperRight.lat = 2.284;
    equatorial_guinea_bb.upperRight.lon = 11.285;
    retn.insert(QLatin1String("Equatorial_Guinea"), equatorial_guinea_bb);

    BoundingBox eritrea_bb;
    eritrea_bb.lowerLeft.lat = 12.455;
    eritrea_bb.lowerLeft.lon = 36.323;
    eritrea_bb.upperRight.lat = 17.998;
    eritrea_bb.upperRight.lon = 43.0812;
    retn.insert(QLatin1String("Eritrea"), eritrea_bb);

    BoundingBox ethiopia_bb;
    ethiopia_bb.lowerLeft.lat = 3.422;
    ethiopia_bb.lowerLeft.lon = 32.954;
    ethiopia_bb.upperRight.lat = 14.959;
    ethiopia_bb.upperRight.lon = 47.789;
    retn.insert(QLatin1String("Ethiopia"), ethiopia_bb);

    BoundingBox gabon_bb;
    gabon_bb.lowerLeft.lat = -3.978;
    gabon_bb.lowerLeft.lon = 8.798;
    gabon_bb.upperRight.lat = 2.327;
    gabon_bb.upperRight.lon = 14.425;
    retn.insert(QLatin1String("Gabon"), gabon_bb);

    BoundingBox gambia_bb;
    gambia_bb.lowerLeft.lat = 13.130;
    gambia_bb.lowerLeft.lon = -16.841;
    gambia_bb.upperRight.lat = 13.876;
    gambia_bb.upperRight.lon = -13.845;
    retn.insert(QLatin1String("Gambia"), gambia_bb);

    BoundingBox ghana_bb;
    ghana_bb.lowerLeft.lat = 4.710;
    ghana_bb.lowerLeft.lon = -3.244;
    ghana_bb.upperRight.lat = 11.098;
    ghana_bb.upperRight.lon = 1.060;
    retn.insert(QLatin1String("Ghana"), ghana_bb);

    BoundingBox guinea_bb;
    guinea_bb.lowerLeft.lat = 7.309;
    guinea_bb.lowerLeft.lon = -15.130;
    guinea_bb.upperRight.lat = 12.586;
    guinea_bb.upperRight.lon = -7.832;
    retn.insert(QLatin1String("Guinea"), guinea_bb);

    BoundingBox guinea_bissau_bb;
    guinea_bissau_bb.lowerLeft.lat = 11.040;
    guinea_bissau_bb.lowerLeft.lon = -16.677;
    guinea_bissau_bb.upperRight.lat = 12.628;
    guinea_bissau_bb.upperRight.lon = -13.700;
    retn.insert(QLatin1String("Guinea_Bissau"), guinea_bissau_bb);

    BoundingBox ivory_coast_bb;
    ivory_coast_bb.lowerLeft.lat = 4.338;
    ivory_coast_bb.lowerLeft.lon = -8.602;
    ivory_coast_bb.upperRight.lat = 10.524;
    ivory_coast_bb.upperRight.lon = -2.562;
    retn.insert(QLatin1String("Ivory_Coast"), ivory_coast_bb);

    BoundingBox kenya_bb;
    kenya_bb.lowerLeft.lat = -4.676;
    kenya_bb.lowerLeft.lon = 33.893;
    kenya_bb.upperRight.lat = 5.506;
    kenya_bb.upperRight.lon = 41.855;
    retn.insert(QLatin1String("Kenya"), kenya_bb);

    BoundingBox lesotho_bb;
    lesotho_bb.lowerLeft.lat = -30.645;
    lesotho_bb.lowerLeft.lon = 26.999;
    lesotho_bb.upperRight.lat = -28.648;
    lesotho_bb.upperRight.lon = 29.325;
    retn.insert(QLatin1String("Lesotho"), lesotho_bb);

    BoundingBox liberia_bb;
    liberia_bb.lowerLeft.lat = 4.356;
    liberia_bb.lowerLeft.lon = -11.439;
    liberia_bb.upperRight.lat = 8.541;
    liberia_bb.upperRight.lon = -7.540;
    retn.insert(QLatin1String("Liberia"), liberia_bb);

    BoundingBox libya_bb;
    libya_bb.lowerLeft.lat = 19.580;
    libya_bb.lowerLeft.lon = 9.319;
    libya_bb.upperRight.lat = 33.137;
    libya_bb.upperRight.lon = 25.165;
    retn.insert(QLatin1String("Libya"), libya_bb);

    BoundingBox madagascar_bb;
    madagascar_bb.lowerLeft.lat = -25.601;
    madagascar_bb.lowerLeft.lon = 43.254;
    madagascar_bb.upperRight.lat = -12.041;
    madagascar_bb.upperRight.lon = 50.477;
    retn.insert(QLatin1String("Madagascar"), madagascar_bb);

    BoundingBox madeira_bb;
    madeira_bb.lowerLeft.lat = 30.028;
    madeira_bb.lowerLeft.lon = -17.266;
    madeira_bb.upperRight.lat = 33.117;
    madeira_bb.upperRight.lon = -15.853;
    retn.insert(QLatin1String("Madeira"), madeira_bb);

    BoundingBox malawi_bb;
    malawi_bb.lowerLeft.lat = -16.801;
    malawi_bb.lowerLeft.lon = 32.688;
    malawi_bb.upperRight.lat = -9.231;
    malawi_bb.upperRight.lon = 35.772;
    retn.insert(QLatin1String("Malawi"), malawi_bb);

    BoundingBox mali_bb;
    mali_bb.lowerLeft.lat = 10.096;
    mali_bb.lowerLeft.lon = -12.170;
    mali_bb.upperRight.lat = 24.975;
    mali_bb.upperRight.lon = 4.270;
    retn.insert(QLatin1String("Mali"), mali_bb);

    BoundingBox mauritania_bb;
    mauritania_bb.lowerLeft.lat = 14.617;
    mauritania_bb.lowerLeft.lon = -17.063;
    mauritania_bb.upperRight.lat = 27.396;
    mauritania_bb.upperRight.lon = -4.923;
    retn.insert(QLatin1String("Mauritania"), mauritania_bb);

    BoundingBox mauritius_bb;
    mauritius_bb.lowerLeft.lat = -20.526;
    mauritius_bb.lowerLeft.lon = 56.513;
    mauritius_bb.upperRight.lat = -10.319;
    mauritius_bb.upperRight.lon = 63.525;
    retn.insert(QLatin1String("Mauritius"), mauritius_bb);

    BoundingBox mayotte_bb;
    mayotte_bb.lowerLeft.lat = -13.000;
    mayotte_bb.lowerLeft.lon = 45.014;
    mayotte_bb.upperRight.lat = -12.633;
    mayotte_bb.upperRight.lon = 45.317;
    retn.insert(QLatin1String("Mayotte"), mayotte_bb);

    BoundingBox melilla_bb;
    melilla_bb.lowerLeft.lat = 35.269;
    melilla_bb.lowerLeft.lon = -2.963;
    melilla_bb.upperRight.lat = 35.307;
    melilla_bb.upperRight.lon = -2.923;
    retn.insert(QLatin1String("Melilla"), melilla_bb);

    BoundingBox morocco_bb;
    morocco_bb.lowerLeft.lat = 21.421;
    morocco_bb.lowerLeft.lon = -17.020;
    morocco_bb.upperRight.lat = 35.760;
    morocco_bb.upperRight.lon = -1.125;
    retn.insert(QLatin1String("Morocco"), morocco_bb);

    BoundingBox mozambique_bb;
    mozambique_bb.lowerLeft.lat = -26.742;
    mozambique_bb.lowerLeft.lon = 30.179;
    mozambique_bb.upperRight.lat = -10.317;
    mozambique_bb.upperRight.lon = 40.775;
    retn.insert(QLatin1String("Mozambique"), mozambique_bb);

    BoundingBox namibia_bb;
    namibia_bb.lowerLeft.lat = -29.045;
    namibia_bb.lowerLeft.lon = 11.734;
    namibia_bb.upperRight.lat = -16.941;
    namibia_bb.upperRight.lon = 25.084;
    retn.insert(QLatin1String("Namibia"), namibia_bb);

    BoundingBox niger_bb;
    niger_bb.lowerLeft.lat = 11.660;
    niger_bb.lowerLeft.lon = 0.296;
    niger_bb.upperRight.lat = 23.472;
    niger_bb.upperRight.lon = 15.903;
    retn.insert(QLatin1String("Niger"), niger_bb);

    BoundingBox nigeria_bb;
    nigeria_bb.lowerLeft.lat = 4.240;
    nigeria_bb.lowerLeft.lon = 2.691;
    nigeria_bb.upperRight.lat = 13.866;
    nigeria_bb.upperRight.lon = 14.577;
    retn.insert(QLatin1String("Nigeria"), nigeria_bb);

    BoundingBox reunion_bb;
    reunion_bb.lowerLeft.lat = -21.372;
    reunion_bb.lowerLeft.lon = 55.219;
    reunion_bb.upperRight.lat = -20.856;
    reunion_bb.upperRight.lon = 55.845;
    retn.insert(QLatin1String("Reunion"), reunion_bb);

    BoundingBox rwanda_bb;
    rwanda_bb.lowerLeft.lat = -2.918;
    rwanda_bb.lowerLeft.lon = 29.024;
    rwanda_bb.upperRight.lat = -1.137;
    rwanda_bb.upperRight.lon = 30.816;
    retn.insert(QLatin1String("Rwanda"), rwanda_bb);

    BoundingBox sao_tome_and_principe_bb;
    sao_tome_and_principe_bb.lowerLeft.lat = -0.014;
    sao_tome_and_principe_bb.lowerLeft.lon = 5.599;
    sao_tome_and_principe_bb.upperRight.lat = 1.734;
    sao_tome_and_principe_bb.upperRight.lon = 7.466;
    retn.insert(QLatin1String("Sao_Tome_and_Principe"), sao_tome_and_principe_bb);

    BoundingBox senegal_bb;
    senegal_bb.lowerLeft.lat = 12.332;
    senegal_bb.lowerLeft.lon = -17.625;
    senegal_bb.upperRight.lat = 16.598;
    senegal_bb.upperRight.lon = -11.467;
    retn.insert(QLatin1String("Senegal"), senegal_bb);

    BoundingBox seychelles_bb;
    seychelles_bb.lowerLeft.lat = -10.217;
    seychelles_bb.lowerLeft.lon = 46.199;
    seychelles_bb.upperRight.lat = -3.711;
    seychelles_bb.upperRight.lon = 56.279;
    retn.insert(QLatin1String("Seychelles"), seychelles_bb);

    BoundingBox sierra_leone_bb;
    sierra_leone_bb.lowerLeft.lat = 6.785;
    sierra_leone_bb.lowerLeft.lon = -13.247;
    sierra_leone_bb.upperRight.lat = 10.047;
    sierra_leone_bb.upperRight.lon = -10.230;
    retn.insert(QLatin1String("Sierra_Leone"), sierra_leone_bb);

    BoundingBox somalia_bb;
    somalia_bb.lowerLeft.lat = -1.683;
    somalia_bb.lowerLeft.lon = 40.981;
    somalia_bb.upperRight.lat = 12.025;
    somalia_bb.upperRight.lon = 51.134;
    retn.insert(QLatin1String("Somalia"), somalia_bb);

    BoundingBox south_africa_bb;
    south_africa_bb.lowerLeft.lat = -34.819;
    south_africa_bb.lowerLeft.lon = 16.345;
    south_africa_bb.upperRight.lat = -22.091;
    south_africa_bb.upperRight.lon = 32.830;
    retn.insert(QLatin1String("South_Africa"), south_africa_bb);

    BoundingBox south_sudan_bb;
    south_sudan_bb.lowerLeft.lat = 3.509;
    south_sudan_bb.lowerLeft.lon = 23.887;
    south_sudan_bb.upperRight.lat = 12.248;
    south_sudan_bb.upperRight.lon = 35.298;
    retn.insert(QLatin1String("South_Sudan"), south_sudan_bb);

    BoundingBox sudan_bb;
    sudan_bb.lowerLeft.lat = 8.620;
    sudan_bb.lowerLeft.lon = 21.937;
    sudan_bb.upperRight.lat = 22.0;
    sudan_bb.upperRight.lon = 38.410;
    retn.insert(QLatin1String("Sudan"), sudan_bb);

    BoundingBox swaziland_bb;
    swaziland_bb.lowerLeft.lat = -27.286;
    swaziland_bb.lowerLeft.lon = 30.676;
    swaziland_bb.upperRight.lat = -25.660;
    swaziland_bb.upperRight.lon = 32.072;
    retn.insert(QLatin1String("Swaziland"), swaziland_bb);

    BoundingBox tanzania_bb;
    tanzania_bb.lowerLeft.lat = -11.720;
    tanzania_bb.lowerLeft.lon = 29.340;
    tanzania_bb.upperRight.lat = -0.95;
    tanzania_bb.upperRight.lon = 40.317;
    retn.insert(QLatin1String("Tanzania"), tanzania_bb);

    BoundingBox togo_bb;
    togo_bb.lowerLeft.lat = 5.929;
    togo_bb.lowerLeft.lon = -0.049;
    togo_bb.upperRight.lat = 11.018;
    togo_bb.upperRight.lon = 1.865;
    retn.insert(QLatin1String("Togo"), togo_bb);

    BoundingBox tunisia_bb;
    tunisia_bb.lowerLeft.lat = 30.307;
    tunisia_bb.lowerLeft.lon = 7.524;
    tunisia_bb.upperRight.lat = 37.350;
    tunisia_bb.upperRight.lon = 11.489;
    retn.insert(QLatin1String("Tunisia"), tunisia_bb);

    BoundingBox uganda_bb;
    uganda_bb.lowerLeft.lat = -1.443;
    uganda_bb.lowerLeft.lon = 29.579;
    uganda_bb.upperRight.lat = 4.249;
    uganda_bb.upperRight.lon = 35.035;
    retn.insert(QLatin1String("Uganda"), uganda_bb);

    BoundingBox zambia_bb;
    zambia_bb.lowerLeft.lat = -17.961;
    zambia_bb.lowerLeft.lon = 21.888;
    zambia_bb.upperRight.lat = -8.238;
    zambia_bb.upperRight.lon = 33.485;
    retn.insert(QLatin1String("Zambia"), zambia_bb);

    BoundingBox zimbabwe_bb;
    zimbabwe_bb.lowerLeft.lat = -22.271;
    zimbabwe_bb.lowerLeft.lon = 25.264;
    zimbabwe_bb.upperRight.lat = -15.508;
    zimbabwe_bb.upperRight.lon = 32.850;
    retn.insert(QLatin1String("Zimbabwe"), zimbabwe_bb);

// Europe

    BoundingBox aland_islands_bb;
    aland_islands_bb.lowerLeft.lat = 59.736;
    aland_islands_bb.lowerLeft.lon = 19.263;
    aland_islands_bb.upperRight.lat = 60.666;
    aland_islands_bb.upperRight.lon = 21.324;
    retn.insert(QLatin1String("Aland_Islands"), aland_islands_bb);

    BoundingBox albania_bb;
    albania_bb.lowerLeft.lat = 39.625;
    albania_bb.lowerLeft.lon = 19.304;
    albania_bb.upperRight.lat = 42.688;
    albania_bb.upperRight.lon = 21.020;
    retn.insert(QLatin1String("Albania"), albania_bb);

    BoundingBox andorra_bb;
    andorra_bb.lowerLeft.lat = 39.644;
    andorra_bb.lowerLeft.lon = 1.414;
    andorra_bb.upperRight.lat = 41.248;
    andorra_bb.upperRight.lon = 1.787;
    retn.insert(QLatin1String("Andorra"), andorra_bb);

    BoundingBox armenia_bb;
    armenia_bb.lowerLeft.lat = 38.741;
    armenia_bb.lowerLeft.lon = 43.583;
    armenia_bb.upperRight.lat = 41.248;
    armenia_bb.upperRight.lon = 46.505;
    retn.insert(QLatin1String("Armenia"), armenia_bb);

    BoundingBox austria_bb;
    austria_bb.lowerLeft.lat = 46.432;
    austria_bb.lowerLeft.lon = 9.480;
    austria_bb.upperRight.lat = 49.039;
    austria_bb.upperRight.lon = 16.980;
    retn.insert(QLatin1String("Austria"), austria_bb);

    BoundingBox azerbaijan_bb;
    azerbaijan_bb.lowerLeft.lat = 38.2704;
    azerbaijan_bb.lowerLeft.lon = 44.794;
    azerbaijan_bb.upperRight.lat = 41.861;
    azerbaijan_bb.upperRight.lon = 50.393;
    retn.insert(QLatin1String("Azerbaijan"), azerbaijan_bb);

    BoundingBox belarus_bb;
    belarus_bb.lowerLeft.lat = 51.320;
    belarus_bb.lowerLeft.lon = 23.199;
    belarus_bb.upperRight.lat = 56.1691;
    belarus_bb.upperRight.lon = 32.694;
    retn.insert(QLatin1String("Belarus"), belarus_bb);

    BoundingBox belgium_bb;
    belgium_bb.lowerLeft.lat = 49.529;
    belgium_bb.lowerLeft.lon = 2.514;
    belgium_bb.upperRight.lat = 51.475;
    belgium_bb.upperRight.lon = 6.157;
    retn.insert(QLatin1String("Belgium"), belgium_bb);

    BoundingBox bosnia_and_herzegovina_bb;
    bosnia_and_herzegovina_bb.lowerLeft.lat = 42.650;
    bosnia_and_herzegovina_bb.lowerLeft.lon = 15.750;
    bosnia_and_herzegovina_bb.upperRight.lat = 45.234;
    bosnia_and_herzegovina_bb.upperRight.lon = 19.600;
    retn.insert(QLatin1String("Bosnia_and_Herzegovina"), bosnia_and_herzegovina_bb);

    BoundingBox bulgaria_bb;
    bulgaria_bb.lowerLeft.lat = 41.234;
    bulgaria_bb.lowerLeft.lon = 22.381;
    bulgaria_bb.upperRight.lat = 44.235;
    bulgaria_bb.upperRight.lon = 28.558;
    retn.insert(QLatin1String("Bulgaria"), bulgaria_bb);

    BoundingBox croatia_bb;
    croatia_bb.lowerLeft.lat = 42.490;
    croatia_bb.lowerLeft.lon = 13.657;
    croatia_bb.upperRight.lat = 46.504;
    croatia_bb.upperRight.lon = 19.390;
    retn.insert(QLatin1String("Croatia"), croatia_bb);

    BoundingBox cyprus_bb;
    cyprus_bb.lowerLeft.lat = 34.572;
    cyprus_bb.lowerLeft.lon = 32.257;
    cyprus_bb.upperRight.lat = 35.173;
    cyprus_bb.upperRight.lon = 34.005;
    retn.insert(QLatin1String("Cyprus"), cyprus_bb);

    BoundingBox czech_republic_bb;
    czech_republic_bb.lowerLeft.lat = 48.555;
    czech_republic_bb.lowerLeft.lon = 12.240;
    czech_republic_bb.upperRight.lat = 51.117;
    czech_republic_bb.upperRight.lon = 18.853;
    retn.insert(QLatin1String("Czech_Republic"), czech_republic_bb);

    BoundingBox denmark_bb;
    denmark_bb.lowerLeft.lat = 54.800;
    denmark_bb.lowerLeft.lon = 8.090;
    denmark_bb.upperRight.lat = 57.7300;
    denmark_bb.upperRight.lon = 12.690;
    retn.insert(QLatin1String("Denmark"), denmark_bb);

    BoundingBox estonia_bb;
    estonia_bb.lowerLeft.lat = 57.475;
    estonia_bb.lowerLeft.lon = 23.340;
    estonia_bb.upperRight.lat = 59.611;
    estonia_bb.upperRight.lon = 28.132;
    retn.insert(QLatin1String("Estonia"), estonia_bb);

    BoundingBox faroe_islands_bb;
    faroe_islands_bb.lowerLeft.lat = 61.395;
    faroe_islands_bb.lowerLeft.lon = -7.681;
    faroe_islands_bb.upperRight.lat = 62.401;
    faroe_islands_bb.upperRight.lon = -6.259;
    retn.insert(QLatin1String("Faroe_Islands"), faroe_islands_bb);

    BoundingBox finland_bb;
    finland_bb.lowerLeft.lat = 59.45;
    finland_bb.lowerLeft.lon = 18.0;
    finland_bb.upperRight.lat = 70.083;
    finland_bb.upperRight.lon = 33.35;
    retn.insert(QLatin1String("Finland"), finland_bb);

    BoundingBox france_bb;
    france_bb.lowerLeft.lat = 2.0534;
    france_bb.lowerLeft.lon = -54.5248;
    france_bb.upperRight.lat = 51.149;
    france_bb.upperRight.lon = 9.560;
    retn.insert(QLatin1String("France"), france_bb);

    BoundingBox georgia_bb;
    georgia_bb.lowerLeft.lat = 41.064;
    georgia_bb.lowerLeft.lon = 39.955;
    georgia_bb.upperRight.lat = 43.553;
    georgia_bb.upperRight.lon = 46.648;
    retn.insert(QLatin1String("Georgia"), georgia_bb);

    BoundingBox germany_bb;
    germany_bb.lowerLeft.lat = 47.302;
    germany_bb.lowerLeft.lon = 5.989;
    germany_bb.upperRight.lat = 54.983;
    germany_bb.upperRight.lon = 15.017;
    retn.insert(QLatin1String("Germany"), germany_bb);

    BoundingBox gibraltar_bb;
    gibraltar_bb.lowerLeft.lat = 36.108;
    gibraltar_bb.lowerLeft.lon = -5.358;
    gibraltar_bb.upperRight.lat = 36.156;
    gibraltar_bb.upperRight.lon = -5.339;
    retn.insert(QLatin1String("Gibraltar"), gibraltar_bb);

    BoundingBox greece_bb;
    greece_bb.lowerLeft.lat = 34.920;
    greece_bb.lowerLeft.lon = 20.150;
    greece_bb.upperRight.lat = 41.827;
    greece_bb.upperRight.lon = 26.604;
    retn.insert(QLatin1String("Greece"), greece_bb);

    BoundingBox guernsey_bb;
    guernsey_bb.lowerLeft.lat = 49.406;
    guernsey_bb.lowerLeft.lon = -2.675;
    guernsey_bb.upperRight.lat = 49.739;
    guernsey_bb.upperRight.lon = -2.164;
    retn.insert(QLatin1String("Guernsey"), guernsey_bb);

    BoundingBox hungary_bb;
    hungary_bb.lowerLeft.lat = 45.759;
    hungary_bb.lowerLeft.lon = 16.202;
    hungary_bb.upperRight.lat = 48.624;
    hungary_bb.upperRight.lon = 22.711;
    retn.insert(QLatin1String("Hungary"), hungary_bb);

    BoundingBox iceland_bb;
    iceland_bb.lowerLeft.lat = 63.496;
    iceland_bb.lowerLeft.lon = -24.326;
    iceland_bb.upperRight.lat = 66.526;
    iceland_bb.upperRight.lon = -13.609;
    retn.insert(QLatin1String("Iceland"), iceland_bb);

    BoundingBox ireland_bb;
    ireland_bb.lowerLeft.lat = 51.669;
    ireland_bb.lowerLeft.lon = -9.977;
    ireland_bb.upperRight.lat = 55.132;
    ireland_bb.upperRight.lon = -6.030;
    retn.insert(QLatin1String("Ireland"), ireland_bb);

    BoundingBox isle_of_man_bb;
    isle_of_man_bb.lowerLeft.lat = 54.045;
    isle_of_man_bb.lowerLeft.lon = -4.830;
    isle_of_man_bb.upperRight.lat = 54.419;
    isle_of_man_bb.upperRight.lon = -4.310;
    retn.insert(QLatin1String("Isle_of_Man"), isle_of_man_bb);

    BoundingBox italy_bb;
    italy_bb.lowerLeft.lat = 36.620;
    italy_bb.lowerLeft.lon = 6.745;
    italy_bb.upperRight.lat = 47.115;
    italy_bb.upperRight.lon = 18.480;
    retn.insert(QLatin1String("Italy"), italy_bb);

    BoundingBox jersey_bb;
    jersey_bb.lowerLeft.lat = 49.162;
    jersey_bb.lowerLeft.lon = -2.255;
    jersey_bb.upperRight.lat = 49.262;
    jersey_bb.upperRight.lon = -2.011;
    retn.insert(QLatin1String("Jersey"), jersey_bb);

    BoundingBox kazakhstan_bb;
    kazakhstan_bb.lowerLeft.lat = 40.663;
    kazakhstan_bb.lowerLeft.lon = 46.466;
    kazakhstan_bb.upperRight.lat = 55.385;
    kazakhstan_bb.upperRight.lon = 87.360;
    retn.insert(QLatin1String("Kazakhstan"), kazakhstan_bb);

    BoundingBox kosovo_bb;
    kosovo_bb.lowerLeft.lat = 42.111;
    kosovo_bb.lowerLeft.lon = 20.787;
    kosovo_bb.upperRight.lat = 43.153;
    kosovo_bb.upperRight.lon = 21.514;
    retn.insert(QLatin1String("Kosovo"), kosovo_bb);

    BoundingBox latvia_bb;
    latvia_bb.lowerLeft.lat = 55.615;
    latvia_bb.lowerLeft.lon = 21.056;
    latvia_bb.upperRight.lat = 57.970;
    latvia_bb.upperRight.lon = 28.176;
    retn.insert(QLatin1String("Latvia"), latvia_bb);

    BoundingBox liechtenstein_bb;
    liechtenstein_bb.lowerLeft.lat = 47.048;
    liechtenstein_bb.lowerLeft.lon = 9.472;
    liechtenstein_bb.upperRight.lat = 47.270;
    liechtenstein_bb.upperRight.lon = 9.636;
    retn.insert(QLatin1String("Liechtenstein"), liechtenstein_bb);

    BoundingBox lithuania_bb;
    lithuania_bb.lowerLeft.lat = 53.906;
    lithuania_bb.lowerLeft.lon = 21.056;
    lithuania_bb.upperRight.lat = 56.373;
    lithuania_bb.upperRight.lon = 26.588;
    retn.insert(QLatin1String("Lithuania"), lithuania_bb);

    BoundingBox luxembourg_bb;
    luxembourg_bb.lowerLeft.lat = 49.443;
    luxembourg_bb.lowerLeft.lon = 5.674;
    luxembourg_bb.upperRight.lat = 50.128;
    luxembourg_bb.upperRight.lon = 6.243;
    retn.insert(QLatin1String("Luxembourg"), luxembourg_bb);

    BoundingBox macedonia_bb;
    macedonia_bb.lowerLeft.lat = 40.843;
    macedonia_bb.lowerLeft.lon = 20.463;
    macedonia_bb.upperRight.lat = 42.320;
    macedonia_bb.upperRight.lon = 22.952;
    retn.insert(QLatin1String("Macedonia"), macedonia_bb);

    BoundingBox malta_bb;
    malta_bb.lowerLeft.lat = 35.786;
    malta_bb.lowerLeft.lon = 14.183;
    malta_bb.upperRight.lat = 36.082;
    malta_bb.upperRight.lon = 14.577;
    retn.insert(QLatin1String("Malta"), malta_bb);

    BoundingBox moldova_bb;
    moldova_bb.lowerLeft.lat = 45.488;
    moldova_bb.lowerLeft.lon = 26.619;
    moldova_bb.upperRight.lat = 48.497;
    moldova_bb.upperRight.lon = 30.025;
    retn.insert(QLatin1String("Moldova"), moldova_bb);

    BoundingBox monaco_bb;
    monaco_bb.lowerLeft.lat = 43.725;
    monaco_bb.lowerLeft.lon = 7.409;
    monaco_bb.upperRight.lat = 43.752;
    monaco_bb.upperRight.lon = 7.439;
    retn.insert(QLatin1String("Monaco"), monaco_bb);

    BoundingBox montenegro_bb;
    montenegro_bb.lowerLeft.lat = 41.878;
    montenegro_bb.lowerLeft.lon = 18.45;
    montenegro_bb.upperRight.lat = 43.524;
    montenegro_bb.upperRight.lon = 20.340;
    retn.insert(QLatin1String("Montenegro"), montenegro_bb);

    BoundingBox netherlands_bb;
    netherlands_bb.lowerLeft.lat = 50.804;
    netherlands_bb.lowerLeft.lon = 3.315;
    netherlands_bb.upperRight.lat = 53.510;
    netherlands_bb.upperRight.lon = 7.092;
    retn.insert(QLatin1String("Netherlands"), netherlands_bb);

    BoundingBox norway_bb;
    norway_bb.lowerLeft.lat = 58.079;
    norway_bb.lowerLeft.lon = 4.992;
    norway_bb.upperRight.lat = 80.657;
    norway_bb.upperRight.lon = 31.293;
    retn.insert(QLatin1String("Norway"), norway_bb);

    BoundingBox poland_bb;
    poland_bb.lowerLeft.lat = 49.027;
    poland_bb.lowerLeft.lon = 14.075;
    poland_bb.upperRight.lat = 54.851;
    poland_bb.upperRight.lon = 24.030;
    retn.insert(QLatin1String("Poland"), poland_bb);

    BoundingBox portugal_bb;
    portugal_bb.lowerLeft.lat = 36.838;
    portugal_bb.lowerLeft.lon = -9.527;
    portugal_bb.upperRight.lat = 42.280;
    portugal_bb.upperRight.lon = -6.389;
    retn.insert(QLatin1String("Portugal"), portugal_bb);

    BoundingBox romania_bb;
    romania_bb.lowerLeft.lat = 43.688;
    romania_bb.lowerLeft.lon = 20.220;
    romania_bb.upperRight.lat = 48.221;
    romania_bb.upperRight.lon = 29.637;
    retn.insert(QLatin1String("Romania"), romania_bb);

    BoundingBox san_marino_bb;
    san_marino_bb.lowerLeft.lat = 43.893;
    san_marino_bb.lowerLeft.lon = 12.403;
    san_marino_bb.upperRight.lat = 43.992;
    san_marino_bb.upperRight.lon = 12.517;
    retn.insert(QLatin1String("San_Marino"), san_marino_bb);

    BoundingBox serbia_bb;
    serbia_bb.lowerLeft.lat = 42.245;
    serbia_bb.lowerLeft.lon = 18.830;
    serbia_bb.upperRight.lat = 46.171;
    serbia_bb.upperRight.lon = 22.986;
    retn.insert(QLatin1String("Serbia"), serbia_bb);

    BoundingBox slovakia_bb;
    slovakia_bb.lowerLeft.lat = 47.758;
    slovakia_bb.lowerLeft.lon = 16.880;
    slovakia_bb.upperRight.lat = 49.571;
    slovakia_bb.upperRight.lon = 22.558;
    retn.insert(QLatin1String("Slovakia"), slovakia_bb);

    BoundingBox slovenia_bb;
    slovenia_bb.lowerLeft.lat = 45.452;
    slovenia_bb.lowerLeft.lon = 13.698;
    slovenia_bb.upperRight.lat = 46.852;
    slovenia_bb.upperRight.lon = 16.565;
    retn.insert(QLatin1String("Slovenia"), slovenia_bb);

    BoundingBox spain_bb;
    spain_bb.lowerLeft.lat = 35.947;
    spain_bb.lowerLeft.lon = -9.393;
    spain_bb.upperRight.lat = 43.748;
    spain_bb.upperRight.lon = 3.039;
    retn.insert(QLatin1String("Spain"), spain_bb);

    BoundingBox sweden_bb;
    sweden_bb.lowerLeft.lat = 55.362;
    sweden_bb.lowerLeft.lon = 11.027;
    sweden_bb.upperRight.lat = 69.106;
    sweden_bb.upperRight.lon = 23.903;
    retn.insert(QLatin1String("Sweden"), sweden_bb);

    BoundingBox switzerland_bb;
    switzerland_bb.lowerLeft.lat = 45.777;
    switzerland_bb.lowerLeft.lon = 6.023;
    switzerland_bb.upperRight.lat = 47.831;
    switzerland_bb.upperRight.lon = 10.443;
    retn.insert(QLatin1String("Switzerland"), switzerland_bb);

    BoundingBox ukraine_bb;
    ukraine_bb.lowerLeft.lat = 44.361;
    ukraine_bb.lowerLeft.lon = 22.057;
    ukraine_bb.upperRight.lat = 52.335;
    ukraine_bb.upperRight.lon = 40.081;
    retn.insert(QLatin1String("Ukraine"), ukraine_bb);

    BoundingBox united_kingdom_bb;
    united_kingdom_bb.lowerLeft.lat = 49.960;
    united_kingdom_bb.lowerLeft.lon = -7.572;
    united_kingdom_bb.upperRight.lat = 58.635;
    united_kingdom_bb.upperRight.lon = 1.681;
    retn.insert(QLatin1String("United_Kingdom"), united_kingdom_bb);

    BoundingBox india_bb;
    india_bb.lowerLeft.lat = 6.0;
    india_bb.lowerLeft.lon = 65.0;
    india_bb.upperRight.lat = 35.956;
    india_bb.upperRight.lon = 97.35;
    retn.insert(QLatin1String("India"), india_bb);

    BoundingBox australia_bb;
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

    QVector<BoundingBox> easterneurope;
    easterneurope << cbboxes.value(QLatin1String("Belarus"))
          << cbboxes.value(QLatin1String("Bulgaria"))
          << cbboxes.value(QLatin1String("Czech_Republic"))
          << cbboxes.value(QLatin1String("Hungary"))
          << cbboxes.value(QLatin1String("Moldova"))
          << cbboxes.value(QLatin1String("Poland"))
          << cbboxes.value(QLatin1String("Romania"))
          << cbboxes.value(QLatin1String("Slovakia"))
          << cbboxes.value(QLatin1String("Ukraine"));
    retn.insert(QLatin1String("Eastern_Europe"), easterneurope);

    QVector<BoundingBox> northerneurope;
    northerneurope << cbboxes.value(QLatin1String("Denmark"))
          << cbboxes.value(QLatin1String("Estonia"))
          << cbboxes.value(QLatin1String("Finland"))
          << cbboxes.value(QLatin1String("Iceland"))
          << cbboxes.value(QLatin1String("Ireland"))
          << cbboxes.value(QLatin1String("Latvia"))
          << cbboxes.value(QLatin1String("Lithuania"))
          << cbboxes.value(QLatin1String("Norway"))
          << cbboxes.value(QLatin1String("Sweden"))
          << cbboxes.value(QLatin1String("United_Kingdom"));
    retn.insert(QLatin1String("Northern_Europe"), northerneurope);

    QVector<BoundingBox> westerneurope;
    westerneurope << cbboxes.value(QLatin1String("Austria"))
          << cbboxes.value(QLatin1String("Belgium"))
          << cbboxes.value(QLatin1String("France"))
          << cbboxes.value(QLatin1String("Germany"))
          << cbboxes.value(QLatin1String("Liechtenstein"))
          << cbboxes.value(QLatin1String("Luxembourg"))
          << cbboxes.value(QLatin1String("Monaco"))
          << cbboxes.value(QLatin1String("Netherlands"))
          << cbboxes.value(QLatin1String("Switzerland"));
    retn.insert(QLatin1String("Western_Europe"), westerneurope);

    QVector<BoundingBox> southerneurope;
    southerneurope << cbboxes.value(QLatin1String("Albania")) 
          << cbboxes.value(QLatin1String("Andorra"))
          << cbboxes.value(QLatin1String("Bosnia_and_Herzegovina"))
          << cbboxes.value(QLatin1String("Croatia"))
          << cbboxes.value(QLatin1String("Greece"))
          << cbboxes.value(QLatin1String("Italy"))
          << cbboxes.value(QLatin1String("Macedonia"))
          << cbboxes.value(QLatin1String("Malta"))
          << cbboxes.value(QLatin1String("Montenegro"))
          << cbboxes.value(QLatin1String("Portugal"))
          << cbboxes.value(QLatin1String("San_Marino"))
          << cbboxes.value(QLatin1String("Serbia"))
          << cbboxes.value(QLatin1String("Slovenia"))
          << cbboxes.value(QLatin1String("Spain"));
    retn.insert(QLatin1String("Southern_Europe"), southerneurope);

    QVector<BoundingBox> europe;
    europe << cbboxes.value(QLatin1String("Albania"))
          << cbboxes.value(QLatin1String("Andorra"))
          << cbboxes.value(QLatin1String("Austria"))
          << cbboxes.value(QLatin1String("Belarus"))
          << cbboxes.value(QLatin1String("Belgium"))
          << cbboxes.value(QLatin1String("Bosnia_and_Herzegovina"))
          << cbboxes.value(QLatin1String("Bulgaria"))
          << cbboxes.value(QLatin1String("Croatia"))
          << cbboxes.value(QLatin1String("Czech_Republic"))
          << cbboxes.value(QLatin1String("Denmark"))
          << cbboxes.value(QLatin1String("Estonia"))
          << cbboxes.value(QLatin1String("Finland"))
          << cbboxes.value(QLatin1String("France"))
          << cbboxes.value(QLatin1String("Germany"))
          << cbboxes.value(QLatin1String("Greece"))
          << cbboxes.value(QLatin1String("Hungary"))
          << cbboxes.value(QLatin1String("Iceland"))
          << cbboxes.value(QLatin1String("Ireland"))
          << cbboxes.value(QLatin1String("Italy"))
          << cbboxes.value(QLatin1String("Latvia"))
          << cbboxes.value(QLatin1String("Liechtenstein"))
          << cbboxes.value(QLatin1String("Lithuania"))
          << cbboxes.value(QLatin1String("Luxembourg"))
          << cbboxes.value(QLatin1String("Macedonia"))
          << cbboxes.value(QLatin1String("Malta"))
          << cbboxes.value(QLatin1String("Moldova"))
          << cbboxes.value(QLatin1String("Monaco"))
          << cbboxes.value(QLatin1String("Montenegro"))
          << cbboxes.value(QLatin1String("Netherlands"))
          << cbboxes.value(QLatin1String("Norway"))
          << cbboxes.value(QLatin1String("Poland"))
          << cbboxes.value(QLatin1String("Portugal"))
          << cbboxes.value(QLatin1String("Romania"))
          << cbboxes.value(QLatin1String("San_Marino"))
          << cbboxes.value(QLatin1String("Serbia"))
          << cbboxes.value(QLatin1String("Slovakia"))
          << cbboxes.value(QLatin1String("Slovenia"))
          << cbboxes.value(QLatin1String("Spain"))
          << cbboxes.value(QLatin1String("Sweden"))
          << cbboxes.value(QLatin1String("Switzerland"))
          << cbboxes.value(QLatin1String("Ukraine"))
          << cbboxes.value(QLatin1String("United_Kingdom"));
    retn.insert(QLatin1String("Europe"), europe);

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
    quint16 mcc = fields[1].isEmpty() ? 0 : fields[1].toUInt();
    quint16 mnc = fields[2].isEmpty() ? 0 : fields[2].toUInt();
    // check to see that there is a cellId associated.  not the case for some UMTS cells.
    quint32 cellId = fields[4].isEmpty() ? 0 : fields[4].toUInt();
    quint32 locationCode = fields[3].isEmpty() ? 0 : fields[3].toUInt();
    if (cellId > 0) {
        // build the cell location.
        result.uniqueCellId = MlsdbUniqueCellId(cellType, cellId, locationCode, mcc, mnc);
        result.loc.lat = fields[7].toDouble();
        result.loc.lon = fields[6].toDouble();
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
                    " Finland, Australia and India.\n"
                    "Other available European regions:\n"
                    " - `Eastern_Europe` contains: Belarus, Bulgaria, Czech Republic, Hungary,"
                    " Moldova, Poland, Romania, Slovakia and Ukraine\n"
                    " - `Northern_Europe` contains: Denmark, Estonia, Finland, Iceland, Ireland,"
                    " Latvia, Lithuania, Norway, Sweden and United_Kingdom\n"
                    " - `Western_Europe` contains: Austria, Belgium, France, Germany,"
                    " Liechtenstein, Luxembourg, Monaco, Netherlands and Switzerland\n"
                    " - `Southern_Europe` contains: Albania, Andorra, Bosnia_and_Herzegovina,"
                    " Croatia, Greece, Italy, Macedonia, Malta, Montenegro, Portugal, San_Marino,"
                    " Serbia, Slovenia and Spain (not including Holy See (Vatican))\n"
                    " - `Europe` contains: Eastern-, Northern-, Western- and Southern Europe\n");
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
