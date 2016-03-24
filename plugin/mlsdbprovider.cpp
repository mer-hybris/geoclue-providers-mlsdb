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
    m_positioningStarted(false),
    m_status(StatusUnavailable),
    m_cellWatcher(new QOfonoExtCellWatcher(this))
{
    if (staticProvider)
        qFatal("Only a single instance of MlsdbProvider is supported.");

    qRegisterMetaType<Location>();
    qDBusRegisterMetaType<Accuracy>();

    staticProvider = this;

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

    cellularNetworkRegistrationChanged();
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
    const QString mlsdbdata(QStringLiteral("/usr/share/geoclue-provider-mlsdb/mlsdb.data"));
    if (!QFile::exists(mlsdbdata)) {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file does not exist:" << mlsdbdata;
        return;
    }

    QFile file(mlsdbdata);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    quint32 magic = 0, expectedMagic = (quint32)0xc710cdb;
    in >> magic;
    if (magic != 0xc710cdb) {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file format unknown:" << magic << "expected:" << expectedMagic;
        return;
    }
    qint32 version;
    in >> version;
    if (version != 1) {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file version unknown:" << version;
        return;
    }
    in >> m_cellIdToLocation;
    if (m_cellIdToLocation.isEmpty()) {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file contained no cell tower locations!";
    } else {
        qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file contains" << m_cellIdToLocation.size() << "cell tower locations.";
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
        qApp->quit();
    } else if (event->timerId() == m_fixLostTimer.timerId()) {
        m_fixLostTimer.stop();
        setStatus(StatusAcquiring);
    } else if (event->timerId() == m_recalculatePositionTimer.timerId()) {
        calculatePositionAndEmitLocation();
    } else {
        QObject::timerEvent(event);
    }
}

void MlsdbProvider::calculatePositionAndEmitLocation()
{
    qCDebug(lcGeoclueMlsdbPosition) << "have" << m_cellWatcher->cells().size() << "neighbouring cells";
    QList<CellPositioningData> cells;
    quint32 maxNeighborSignalStrength = 1;
    QSet<quint32> seenCellIds;
    Q_FOREACH (const QSharedPointer<QOfonoExtCell> &c, m_cellWatcher->cells()) {
        CellPositioningData cell;
        if (c->cid() != -1 && c->cid() != 0) {
            // gsm / wcdma
            cell.cellId = c->cid();
        } else if (c->ci() != -1 && c->ci() != 0) {
            // lte
            cell.cellId = c->ci();
        } else {
            qCDebug(lcGeoclueMlsdbPosition) << "ignoring neighbour cell with no cell id:" << c->type() << c->mcc() << c->mnc();
            continue; // no cell id.
        }
        if (!seenCellIds.contains(cell.cellId)) {
            qCDebug(lcGeoclueMlsdbPosition) << "have neighbour cell:" << cell.cellId << "with strength:" << c->signalStrength();
            cell.signalStrength = c->signalStrength();
            if (cell.signalStrength > maxNeighborSignalStrength) {
                // used for the cell towers we're connected to.
                // if no signal strength data is available from ofono,
                // we assume they're at least as strong signals as the
                // strongest of our neighbor cells.
                maxNeighborSignalStrength = cell.signalStrength;
            }
            cells.append(cell);
            seenCellIds.insert(cell.cellId);
        }
    }

    // determine which towers we have an accurate location for, from MLSDB data.
    double totalSignalStrength = 0.0;
    QMap<quint32, MlsdbCoords> towerLocations;
    Q_FOREACH (const CellPositioningData &cell, cells) {
        if (m_cellIdToLocation.contains(cell.cellId)) {
            MlsdbCoords towerCoords = m_cellIdToLocation.value(cell.cellId);
            towerLocations.insert(cell.cellId, towerCoords);
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
        if (towerLocations.contains(cell.cellId)) {
            const MlsdbCoords &towerCoords(towerLocations.value(cell.cellId));
            double weight = (((double)cell.signalStrength) / totalSignalStrength);
            deviceLatitude += (weight * towerCoords.lat);
            deviceLongitude += (weight * towerCoords.lon);
            qCDebug(lcGeoclueMlsdbPosition) << "have cell tower" << cell.cellId
                                            << "with position:" << towerCoords.lat << "," << towerCoords.lon
                                            << "with strength:" << ((double)cell.signalStrength / totalSignalStrength);
        } else {
            qCDebug(lcGeoclueMlsdbPosition) << "do not know position of cell tower with id:" << cell.cellId;
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

void MlsdbProvider::locationEnabledChanged()
{
    if (positioningEnabled()) {
        startPositioningIfNeeded();
    } else {
        setLocation(Location());
        stopPositioningIfNeeded();
    }
}

void MlsdbProvider::cellularNetworkRegistrationChanged()
{
    qCDebug(lcGeoclueMlsdb);
    calculatePositionAndEmitLocation();
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
    if (!positioningEnabled())
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
    if (positioningEnabled() && !m_watchedServices.isEmpty())
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
    bool cellIdPositioningEnabled = settings.value(QStringLiteral("cell_id_positioning_enabled"), false).toBool();
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
