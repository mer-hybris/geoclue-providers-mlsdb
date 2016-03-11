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
#include <QVariant>
#include <QDataStream>
#include <QFile>

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

    BoundingBox finland_bb;
    finland_bb.latShift = 0.0;
    finland_bb.lonShift = 0.0;
    finland_bb.lowerLeft.lat = 59.45;
    finland_bb.lowerLeft.lon = 18.0;
    finland_bb.upperRight.lat = 70.083;
    finland_bb.upperRight.lon = 33.35;
    retn.insert(QLatin1String("Finland"), finland_bb);

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
    quint32 cellId;
    MlsdbCoords loc;
};
ParseLineResult parseLineAndTest(const QByteArray &line, const QVector<BoundingBox> &boundingBoxes)
{
    ParseLineResult result;
    result.withinBBox = false;

    // radio,mcc,net,area,cell,unit,lon,lat,range,samples,changeable,created,updated,averageSignal
    QList<QByteArray> fields = line.split(',');
    // check to see that there is a cellId associated.  not the case for UMTS towers.
    quint32 cellId = QString::fromLatin1(fields[4]).isEmpty() ? 0 : QString::fromLatin1(fields[4]).toUInt();
    if (cellId > 0) {
        // build the cell tower location.
        result.cellId = cellId;
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

QMap<quint32, MlsdbCoords> siftMlsCsvData(const QString &mlscsv, const QVector<BoundingBox> &boundingBoxes)
{
    QMap<quint32, MlsdbCoords> cellIdToMlsdbCoords;

    // read the csv file line by line, check whether it belongs in the map, insert or discard.
    QFile file(mlscsv);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fprintf(stderr, "Failed to open Mozilla Location Services data .csv file for reading!\n");
        return cellIdToMlsdbCoords;
    }

    fprintf(stdout, "Reading data from file and caching cell tower locations which are within the bounding boxes...\n");
    quint32 readlines = 0, insertedcells = 0, progresscount = 0;
#if USE_CONCURRENT
    int currentJobs = THREADCOUNT-1;
#if USE_STD_CONCURRENT
    std::vector<std::future<ParseLineResult> > results;
#else
    QList<QFuture<ParseLineResult> > results;
#endif // USE_STD_CONCURRENT
#endif // USE_CONCURRENT
    fprintf(stdout, "Progress: %d lines read, %d cell tower locations inserted", readlines, insertedcells);
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
                cellIdToMlsdbCoords.insert(r.cellId, r.loc);
            }
            progresscount++;
        }

        if (progresscount >= 10000) {
            progresscount = 0;
            fprintf(stdout, "\33[2K\rProgress: %d lines read, %d cell tower locations inserted", readlines, insertedcells); // line overwrite
            fflush(stdout);
        }
    }

    fprintf(stdout, "\nFinished reading data: %d lines read, %d cell tower locations inserted\n", readlines, insertedcells);
    return cellIdToMlsdbCoords;
}

int writeMlsdbData(const QMap<quint32, MlsdbCoords> &cellIdToLocation)
{
    if (cellIdToLocation.isEmpty()) {
        fprintf(stderr, "No cell tower locations found which match the required bounding boxes!\n");
        return 1;
    }

    fprintf(stdout, "Writing data to mlsdb.data file...\n");
    QFile file(QLatin1String("mlsdb.data"));
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out << (quint32)0xc710cdb; // magic cell-tower location db number
    out << (qint32)1;          // data file version number
    out.setVersion(QDataStream::Qt_5_0);
    out << cellIdToLocation;

    fprintf(stdout, "Done!\n");
    return 0; // success
}

int queryCellTowerLocation(quint32 cellId, const QString &mlsdbdata)
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
    if (version != 1) {
        fprintf(stderr, "geoclue-mlsdb data file version unknown: %d\n", version);
        return 1;
    }
    QMap<quint32, MlsdbCoords> cellIdToLocation;
    in >> cellIdToLocation;
    if (cellIdToLocation.isEmpty()) {
        fprintf(stderr, "geoclue-mlsdb data file contained no cell tower locations!\n");
        return 1;
    }

    fprintf(stdout, "Searching for %d within data...\n", cellId);
    if (!cellIdToLocation.contains(cellId)) {
        fprintf(stderr, "geoclue-mlsdb data file does not contain location of cell tower with id: %d\n", cellId);
        return 1;
    }

    MlsdbCoords towerLocation = cellIdToLocation.value(cellId);
    fprintf(stdout, "Cell tower with id %d is at lat,lon: %f, %f\n", cellId, towerLocation.lat, towerLocation.lon);
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
                    " country- or region-specific Cell Tower information.\n");
    fprintf(stdout, "Usage:\n\tgeoclue-mlsdb-tool --help [country|region]\n"
                    "\tgeoclue-mlsdb-tool -c <country> mls.csv\n"
                    "\tgeoclue-mlsdb-tool -r <region> mls.csv\n");
    fprintf(stdout, "For more information about valid country and region arguments,"
                    " run `geoclue-mlsdb-tool --help country` or `geoclue-mlsdb-tool --help region`.\n");
}

void printCountryHelp()
{
    fprintf(stdout, "Running `geoclue-mlsdb-tool -c <country> mls.csv` will generate an output"
                    " file `mlsdb.data` which contains a mapping from (uint32) cell"
                    " tower id to (double,double) latitude,longitude for all cell"
                    " towers known in the mls.csv within the bounding-box for that"
                    " country.\n");
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
                    " file `mlsdb.data` which contains a mapping from (uint32) cell"
                    " tower id to (double,double) latitude,longitude for all cell"
                    " towers known in the mls.csv within the bounding-box for that"
                    " region.\n"
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
                    "\tgeoclue-mlsdb-tool --query <cellId> mlsdb.data\n");
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
    } else if (args.length() == 4 && args[1].compare(QStringLiteral("--query"), Qt::CaseInsensitive) == 0) {
        return queryCellTowerLocation(args[2].toUInt(), args[3]);
    }

    printUsageError();
    return 1;
}
