/*
    Copyright (C) 2016 Jolla Ltd.
    Contact: Chris Adams <chris.adams@jollamobile.com>

    This file is part of geoclue-mlsdb.

    Geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#include "mlsdbprovider.h"

#include "geoclue_adaptor.h"
#include "position_adaptor.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfoList>
#include <QtCore/QSharedPointer>
#include <QtCore/QList>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

#include <qofonoextcellwatcher.h>

#include <strings.h>
#include <sys/time.h>

Q_LOGGING_CATEGORY(lcGeoclueMlsdb, "geoclue.provider.mlsdb")
Q_LOGGING_CATEGORY(lcGeoclueMlsdbPosition, "geoclue.provider.mlsdb.position")

namespace {
    MlsdbProvider *staticProvider = 0;
    const int QuitIdleTime = 30000;             // 30s
    const int FixTimeout = 30000;               // 30s
    const quint32 MinimumInterval = 10000;      // 10s
    const quint32 PreferredInitialFixTime = 0;  //  0s
    const QString LocationSettingsFile = QStringLiteral("/etc/location/location.conf");
}

QDBusArgument &operator<<(QDBusArgument &argument, const Accuracy &accuracy)
{
    const qint32 GeoclueAccuracyLevelPostalcode = 4;

    argument.beginStructure();
    argument << GeoclueAccuracyLevelPostalcode << accuracy.horizontal() << accuracy.vertical();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Accuracy &accuracy)
{
    qint32 level;
    double a;

    argument.beginStructure();
    argument >> level;
    argument >> a;
    accuracy.setHorizontal(a);
    argument >> a;
    accuracy.setVertical(a);
    argument.endStructure();
    return argument;
}

MlsdbProvider::MlsdbProvider(QObject *parent)
:   QObject(parent),
    m_positioningEnabled(false),
    m_positioningStarted(false),
    m_status(StatusUnavailable),
    m_cellWatcher(new QOfonoExtCellWatcher(this))
{
    if (staticProvider)
        qFatal("Only a single instance of MlsdbProvider is supported.");

    qRegisterMetaType<Location>();
    qDBusRegisterMetaType<Accuracy>();

    staticProvider = this;

    connect(&m_locationSettingsWatcher, &QFileSystemWatcher::fileChanged,
            this, &MlsdbProvider::updatePositioningEnabled);
    m_locationSettingsWatcher.addPath(LocationSettingsFile);
    updatePositioningEnabled();

    new GeoclueAdaptor(this);
    new PositionAdaptor(this);

    qCDebug(lcGeoclueMlsdb) << "Mozilla Location Services geoclue plugin active";
    populateCellIdToLocationMap();
    m_idleTimer.start(QuitIdleTime, this);

    QDBusConnection connection = QDBusConnection::sessionBus();
    m_watcher = new QDBusServiceWatcher(this);
    m_watcher->setConnection(connection);
    m_watcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &MlsdbProvider::serviceUnregistered);

    connect(m_cellWatcher, &QOfonoExtCellWatcher::cellsChanged,
            this, &MlsdbProvider::cellularNetworkRegistrationChanged);

    if (m_positioningEnabled) {
        cellularNetworkRegistrationChanged();
    } else {
        qCDebug(lcGeoclueMlsdb) << "positioning is not currently enabled, idling";
    }
}

MlsdbProvider::~MlsdbProvider()
{
    if (staticProvider == this)
        staticProvider = 0;
}

/*
 * NOTE: This is VERY memory hungry.
 * TODO: Use a paginated serialisation format to minimise memory use.
 */
void MlsdbProvider::populateCellIdToLocationMap()
{
    QString mlsdbdata(QStringLiteral("/usr/share/geoclue-provider-mlsdb/mlsdb.data"));
    if (!QFile::exists(mlsdbdata)) {
        // look for a country or region specific mlsdb.data file.
        QDir mlsdbdataDir(QStringLiteral("/usr/share/geoclue-provider-mlsdb/"));
        if (!mlsdbdataDir.exists()) {
            return;
        }
        QStringList subdirs = mlsdbdataDir.entryList(QDir::AllDirs | QDir::NoDot | QDir::NoDotDot);
        bool foundSubdirData = false;
        Q_FOREACH (const QString &subdir, subdirs) {
            mlsdbdata = QStringLiteral("/usr/share/geoclue-provider-mlsdb/%1/mlsdb.data").arg(subdir);
            foundSubdirData = true;
            break;
        }
        if (!foundSubdirData) {
            qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file does not exist.";
            return; // no country or region specific data file exists.
        }
    }

    QFile file(mlsdbdata);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    quint32 magic = 0, expectedMagic = (quint32)0xc710cdb;
    in >> magic;
    if (magic != 0xc710cdb) {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << mlsdbdata << "format unknown:" << magic << "expected:" << expectedMagic;
        return;
    }
    qint32 version;
    in >> version;
    if (version != 2) {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << mlsdbdata << "version unknown:" << version;
        return;
    }
    in >> m_composedCellIdToLocation;
    if (m_composedCellIdToLocation.isEmpty()) {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << mlsdbdata << "contained no cell tower locations!";
    } else {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << mlsdbdata << "contains" << m_composedCellIdToLocation.size() << "cell tower locations.";
    }
}

void MlsdbProvider::AddReference()
{
    if (!calledFromDBus())
        qFatal("AddReference must only be called from DBus");

    const QString service = message().service();
    m_watcher->addWatchedService(service);
    m_watchedServices[service].referenceCount += 1;

    startPositioningIfNeeded();
}

void MlsdbProvider::RemoveReference()
{
    if (!calledFromDBus())
        qFatal("RemoveReference must only be called from DBus");

    const QString service = message().service();

    if (m_watchedServices[service].referenceCount > 0)
        m_watchedServices[service].referenceCount -= 1;

    if (m_watchedServices[service].referenceCount == 0) {
        m_watcher->removeWatchedService(service);
        m_watchedServices.remove(service);
    }

    if (m_watchedServices.isEmpty())
        m_idleTimer.start(QuitIdleTime, this);

    stopPositioningIfNeeded();
}

QString MlsdbProvider::GetProviderInfo(QString &description)
{
    description = tr("Mozilla Location Service Database cell-id position provider");
    return QLatin1String("Mlsdb");
}

int MlsdbProvider::GetStatus()
{
    return m_status;
}

void MlsdbProvider::SetOptions(const QVariantMap &options)
{
    if (!calledFromDBus())
        qFatal("SetOptions must only be called from DBus");

    const QString service = message().service();
    if (!m_watchedServices.contains(service)) {
        qWarning("Only active users can call SetOptions");
        return;
    }

    if (options.contains(QStringLiteral("UpdateInterval"))) {
        m_watchedServices[service].updateInterval =
            options.value(QStringLiteral("UpdateInterval")).toUInt();

        quint32 updateInterval = minimumRequestedUpdateInterval();
        m_recalculatePositionTimer.start(updateInterval, this);
    }
}

int MlsdbProvider::GetPosition(int &timestamp, double &latitude, double &longitude,
                                double &altitude, Accuracy &accuracy)
{
    if (m_currentLocation.timestamp() > 0) {
        qCDebug(lcGeoclueMlsdbPosition) << "GetPosition:"
                                        << "timestamp:" << m_currentLocation.timestamp()
                                        << "latitude:" << m_currentLocation.latitude()
                                        << "longitude:" << m_currentLocation.longitude()
                                        << "accuracy:" << m_currentLocation.accuracy().horizontal();
    } else {
        qCDebug(lcGeoclueMlsdbPosition) << "GetPosition: no valid current location known";
    }

    PositionFields positionFields = NoPositionFields;

    timestamp = m_currentLocation.timestamp() / 1000;
    if (!qIsNaN(m_currentLocation.latitude()))
        positionFields |= LatitudePresent;
    latitude = m_currentLocation.latitude();
    if (!qIsNaN(m_currentLocation.longitude()))
        positionFields |= LongitudePresent;
    longitude = m_currentLocation.longitude();
    if (!qIsNaN(m_currentLocation.altitude()))
        positionFields |= AltitudePresent;
    altitude = m_currentLocation.altitude();
    accuracy = m_currentLocation.accuracy();

    return positionFields;
}

void MlsdbProvider::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_idleTimer.timerId()) {
        m_idleTimer.stop();
        qCDebug(lcGeoclueMlsdb) << "have been idle for too long, quitting";
        qApp->quit();
    } else if (event->timerId() == m_fixLostTimer.timerId()) {
        m_fixLostTimer.stop();
        setStatus(StatusAcquiring);
    } else if (event->timerId() == m_recalculatePositionTimer.timerId()) {
        if (m_positioningEnabled) {
            calculatePositionAndEmitLocation();
        }
    } else {
        QObject::timerEvent(event);
    }
}

void MlsdbProvider::calculatePositionAndEmitLocation()
{
    qCDebug(lcGeoclueMlsdbPosition) << "have" << m_cellWatcher->cells().size() << "neighbouring cells";
    QList<CellPositioningData> cells;
    quint32 maxNeighborSignalStrength = 1;
    QSet<quint64> seenCellIds;
    Q_FOREACH (const QSharedPointer<QOfonoExtCell> &c, m_cellWatcher->cells()) {
        CellPositioningData cell;
        MlsdbCellType cellType = MLSDB_CELL_TYPE_LTE;
        quint32 locationCode = 0;
        quint32 cellId = 0;
        if (c->cid() != -1 && c->cid() != 0) {
            // gsm / wcdma
            cellType = MLSDB_CELL_TYPE_GSM_WCDMA;
            locationCode = static_cast<quint32>(c->lac());
            cellId = static_cast<quint32>(c->cid());
        } else if (c->ci() != -1 && c->ci() != 0) {
            // lte
            cellType = MLSDB_CELL_TYPE_LTE;
            locationCode = static_cast<quint32>(c->tac());
            cellId = static_cast<quint32>(c->ci());
        } else {
            qCDebug(lcGeoclueMlsdbPosition) << "ignoring neighbour cell with no cell id:" << c->type() << c->mcc() << c->mnc() << c->lac() << c->tac();
            continue; // no cell id.
        }
        cell.composedCellId = composeMlsdbCellId(cellType, locationCode, cellId);
        if (!seenCellIds.contains(cell.composedCellId)) {
            qCDebug(lcGeoclueMlsdbPosition) << "have neighbour cell:" << stringForMlsdbCellType(cellType) << locationCode << cellId << "with strength:" << c->signalStrength();
            cell.signalStrength = c->signalStrength();
            if (cell.signalStrength > maxNeighborSignalStrength) {
                // used for the cell towers we're connected to.
                // if no signal strength data is available from ofono,
                // we assume they're at least as strong signals as the
                // strongest of our neighbor cells.
                maxNeighborSignalStrength = cell.signalStrength;
            }
            cells.append(cell);
            seenCellIds.insert(cell.composedCellId);
        }
    }

    // determine which towers we have an accurate location for, from MLSDB data.
    double totalSignalStrength = 0.0;
    QMap<quint64, MlsdbCoords> towerLocations;
    Q_FOREACH (const CellPositioningData &cell, cells) {
        if (m_composedCellIdToLocation.contains(cell.composedCellId)) {
            MlsdbCoords towerCoords = m_composedCellIdToLocation.value(cell.composedCellId);
            towerLocations.insert(cell.composedCellId, towerCoords);
            totalSignalStrength += (1.0 * cell.signalStrength);
        }
    }

    if (towerLocations.size() == 0) {
        qCDebug(lcGeoclueMlsdbPosition) << "no cell id data to calculate position from";
        return;
    } else if (towerLocations.size() == 1) {
        qCDebug(lcGeoclueMlsdbPosition) << "only one cell id datum to calculate position from, position will be extremely inaccurate";
    } else if (towerLocations.size() == 2) {
        qCDebug(lcGeoclueMlsdbPosition) << "only two cell id data to calculate position from, position will be highly inaccurate";
    } else {
        qCDebug(lcGeoclueMlsdbPosition) << "calculating position from" << towerLocations.size() << "cell id data";
    }

    // now use the current cell and neighboringcell information to triangulate our position.
    double deviceLatitude = 0.0;
    double deviceLongitude = 0.0;
    Q_FOREACH (const CellPositioningData &cell, cells) {
        quint32 dcellId = 0, dlocationCode = 0;
        MlsdbCellType dcellType = MLSDB_CELL_TYPE_OTHER;
        decomposeMlsdbCellId(cell.composedCellId, &dcellType, &dlocationCode, &dcellId);
        if (towerLocations.contains(cell.composedCellId)) {
            const MlsdbCoords &towerCoords(towerLocations.value(cell.composedCellId));
            double weight = (((double)cell.signalStrength) / totalSignalStrength);
            deviceLatitude += (weight * towerCoords.lat);
            deviceLongitude += (weight * towerCoords.lon);
            qCDebug(lcGeoclueMlsdbPosition) << "have cell tower:" << stringForMlsdbCellType(dcellType) << dlocationCode << dcellId
                                            << "with position:"   << towerCoords.lat << "," << towerCoords.lon
                                            << "with strength:"   << ((double)cell.signalStrength / totalSignalStrength);
        } else {
            qCDebug(lcGeoclueMlsdbPosition) << "do not know position of cell tower with id:"
                                            << stringForMlsdbCellType(dcellType) << dlocationCode << dcellId;
        }
    }

    Location deviceLocation;
    if (towerLocations.size()) {
        // estimate accuracy based on how many cell towers we have.
        Accuracy positionAccuracy;
        positionAccuracy.setHorizontal(qMax(250, (10000 - (1000 * towerLocations.size()))));
        deviceLocation.setTimestamp(QDateTime::currentMSecsSinceEpoch());
        deviceLocation.setLatitude(deviceLatitude);
        deviceLocation.setLongitude(deviceLongitude);
        deviceLocation.setAccuracy(positionAccuracy);
    }

    // and set this as our location.
    setLocation(deviceLocation);
}

void MlsdbProvider::setLocation(const Location &location)
{
    qCDebug(lcGeoclueMlsdbPosition) << "setting current location to:"
                                    << "ts:" << location.timestamp() << ","
                                    << "lat:" << location.latitude() << "," << "lon:" << location.longitude() << ","
                                    << "accuracy:" << location.accuracy().horizontal();

    if (location.timestamp() != 0) {
        setStatus(StatusAvailable);
        m_fixLostTimer.start(FixTimeout, this);
        m_lastLocation = m_currentLocation;
    } else {
        qCDebug(lcGeoclueMlsdbPosition) << "location invalid, lost positioning fix";
        m_lastLocation = Location(); // lost fix, reset last location also.
    }

    m_currentLocation = location;
    emitLocationChanged();
}

void MlsdbProvider::serviceUnregistered(const QString &service)
{
    m_watchedServices.remove(service);
    m_watcher->removeWatchedService(service);

    if (m_watchedServices.isEmpty())
        m_idleTimer.start(QuitIdleTime, this);

    stopPositioningIfNeeded();
}

void MlsdbProvider::updatePositioningEnabled()
{
    bool previous = m_positioningEnabled;
    bool enabled = positioningEnabled();
    if (previous == enabled) {
        // the change to the location settings file doesn't affect this plugin.
        return;
    }

    if (enabled) {
        qCDebug(lcGeoclueMlsdb) << "cellId-based positioning has been enabled";
        m_positioningEnabled = true;
        startPositioningIfNeeded();
    } else {
        qCDebug(lcGeoclueMlsdb) << "cellId-based positioning has been disabled";
        m_positioningEnabled = false;
        setLocation(Location());
        stopPositioningIfNeeded();
    }
}

void MlsdbProvider::cellularNetworkRegistrationChanged()
{
    if (m_positioningEnabled) {
        qCDebug(lcGeoclueMlsdb) << "cellular network registrations changed, updating position";
        calculatePositionAndEmitLocation();
    }
}

void MlsdbProvider::emitLocationChanged()
{
    PositionFields positionFields = NoPositionFields;

    if (!qIsNaN(m_currentLocation.latitude()))
        positionFields |= LatitudePresent;
    if (!qIsNaN(m_currentLocation.longitude()))
        positionFields |= LongitudePresent;
    if (!qIsNaN(m_currentLocation.altitude()))
        positionFields |= AltitudePresent;

    emit PositionChanged(positionFields, m_currentLocation.timestamp() / 1000,
                         m_currentLocation.latitude(), m_currentLocation.longitude(),
                         m_currentLocation.altitude(), m_currentLocation.accuracy());
}

void MlsdbProvider::startPositioningIfNeeded()
{
    // Positioning is already started.
    if (m_positioningStarted)
        return;

    // Positioning is unused.
    if (m_watchedServices.isEmpty())
        return;

    // Positioning disabled externally
    if (!m_positioningEnabled)
        return;

    m_idleTimer.stop();

    qCDebug(lcGeoclueMlsdb) << "Starting positioning";
    m_positioningStarted = true;
    calculatePositionAndEmitLocation();
    quint32 updateInterval = minimumRequestedUpdateInterval();
    m_recalculatePositionTimer.start(updateInterval, this);
}

void MlsdbProvider::stopPositioningIfNeeded()
{
    // Positioning is already stopped.
    if (!m_positioningStarted)
        return;

    // Positioning enabled externally and positioning is still being used.
    if (m_positioningEnabled && !m_watchedServices.isEmpty())
        return;

    qCDebug(lcGeoclueMlsdb) << "Stopping positioning";
    m_positioningStarted = false;
    setStatus(StatusUnavailable);
    m_fixLostTimer.stop();
    m_recalculatePositionTimer.stop();
}

void MlsdbProvider::setStatus(MlsdbProvider::Status status)
{
    if (m_status == status)
        return;

    m_status = status;
    emit StatusChanged(m_status);
}

/*
    Returns true if positioning is enabled, otherwise returns false.

    Currently checks the state of the Location enabled setting and
    the cell_id_positioning_enabled setting.
*/
bool MlsdbProvider::positioningEnabled()
{
    QSettings settings(LocationSettingsFile, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("location"));
    bool enabled = settings.value(QStringLiteral("enabled"), false).toBool();
    bool cellIdPositioningEnabled = settings.value(QStringLiteral("cell_id_positioning_enabled"), true).toBool();
    return enabled && cellIdPositioningEnabled;
}

quint32 MlsdbProvider::minimumRequestedUpdateInterval() const
{
    quint32 updateInterval = UINT_MAX;

    foreach (const ServiceData &data, m_watchedServices) {
        // Old data, service not currently using positioning.
        if (data.referenceCount <= 0) {
            qWarning("Service data was not removed!");
            continue;
        }

        // Service hasn't requested a specific update interval.
        if (data.updateInterval == 0)
            continue;

        updateInterval = qMin(updateInterval, data.updateInterval);
    }

    if (updateInterval == UINT_MAX)
        return MinimumInterval;

    return qMax(updateInterval, MinimumInterval);
}
