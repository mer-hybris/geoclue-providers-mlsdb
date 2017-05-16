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

#include "mlsdbonlinelocator.h"
#include "geoclue_adaptor.h"
#include "position_adaptor.h"

#include <QtGlobal>
#include <QtCore/QLoggingCategory>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFileInfoList>
#include <QtCore/QSharedPointer>
#include <QtCore/QList>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

#include <qofonoextcellwatcher.h>

#include <strings.h>
#include <sys/time.h>

Q_LOGGING_CATEGORY(lcGeoclueMlsdb, "geoclue.provider.mlsdb", QtWarningMsg)
Q_LOGGING_CATEGORY(lcGeoclueMlsdbPosition, "geoclue.provider.mlsdb.position", QtWarningMsg)

namespace {
    MlsdbProvider *staticProvider = 0;
    const int QuitIdleTime = 30000;             // 30s
    const int FixTimeout = 30000;               // 30s
    const quint32 MinimumInterval = 10000;      // 10s
    const quint32 PreferredInitialFixTime = 0;  //  0s
    const QString LocationSettingsDir = QStringLiteral("/etc/location/");
    const QString LocationSettingsFile = QStringLiteral("/etc/location/location.conf");
    const QString LocationSettingsEnabledKey = QStringLiteral("location/enabled");
    const QString LocationSettingsMlsEnabledKey = QStringLiteral("location/mls/enabled");
    const QString LocationSettingsMlsOnlineEnabledKey = QStringLiteral("location/mls/online_enabled");
    const QString LocationSettingsOldMlsEnabledKey = QStringLiteral("location/cell_id_positioning_enabled"); // deprecated key
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
    m_mlsdbOnlineLocator(0),
    m_onlinePositioningEnabled(false),
    m_cellWatcher(new QOfonoExtCellWatcher(this))
{
    if (staticProvider)
        qFatal("Only a single instance of MlsdbProvider is supported.");

    qRegisterMetaType<Location>();
    qDBusRegisterMetaType<Accuracy>();

    staticProvider = this;

    connect(&m_locationSettingsWatcher, &QFileSystemWatcher::fileChanged,
            this, &MlsdbProvider::updatePositioningEnabled);
    connect(&m_locationSettingsWatcher, &QFileSystemWatcher::directoryChanged,
            this, &MlsdbProvider::updatePositioningEnabled);
    m_locationSettingsWatcher.addPath(LocationSettingsDir);
    m_locationSettingsWatcher.addPath(LocationSettingsFile);
    updatePositioningEnabled();

    new GeoclueAdaptor(this);
    new PositionAdaptor(this);

    qCDebug(lcGeoclueMlsdb) << "Mozilla Location Services geoclue plugin active";
    if (m_watchedServices.isEmpty()) {
        m_idleTimer.start(QuitIdleTime, this);
    }

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

/* TODO: coalesce lookups to avoid unnecessary repeated file I/O */
bool MlsdbProvider::searchForCellIdLocation(const MlsdbUniqueCellId &uniqueCellId, MlsdbCoords *coords)
{
    // try to find the mlsdb data file which should contain it.
    // the mlsdb data files are separated into "first digit of location code" directories/buckets.
    QChar firstDigitAreaCode = QString::number(uniqueCellId.locationCode()).at(0);
    QDirIterator it("/usr/share/geoclue-provider-mlsdb/", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString fname(it.next());
        if (fname.endsWith(QStringLiteral("/%1/mlsdb.data").arg(firstDigitAreaCode), Qt::CaseInsensitive)) {
            // found an mlsdb.data file which might contain the cell data.  search it.
            QFile file(fname);
            file.open(QIODevice::ReadOnly);
            QDataStream in(&file);
            quint32 magic = 0, expectedMagic = (quint32)0xc710cdb;
            in >> magic;
            if (magic != 0xc710cdb) {
                qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << fname << "format unknown:" << magic << "expected:" << expectedMagic;
                continue; // ignore this file
            }
            qint32 version;
            in >> version;
            if (version != 3) {
                qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << fname << "version unknown:" << version;
                continue; // ignore this file
            }

            QMap<MlsdbUniqueCellId, MlsdbCoords> perLcCellIdToLocations;
            in >> perLcCellIdToLocations;
            if (perLcCellIdToLocations.isEmpty()) {
                qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << fname << "contained no cell locations!";
            } else {
                if (perLcCellIdToLocations.contains(uniqueCellId)) {
                    *coords = perLcCellIdToLocations.value(uniqueCellId);
                    qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << fname << "contains the location of composed cell id:" << uniqueCellId.toString() << "->" << coords->lat << "," << coords->lon;
                    return true; // found!
                } else {
                    qCDebug(lcGeoclueMlsdb) << "geoclue-mlsdb data file" << fname << "contains" << perLcCellIdToLocations.size() << "cell locations, but not for:" << uniqueCellId.toString();
                }
            }
        }
    }

    qCDebug(lcGeoclueMlsdb) << "no geoclue-mlsdb data files contain the location of composed cell id:" << uniqueCellId.toString();
    return false;
}

void MlsdbProvider::AddReference()
{
    if (!calledFromDBus())
        qFatal("AddReference must only be called from DBus");

    bool wasInactive = m_watchedServices.isEmpty();
    const QString service = message().service();
    m_watcher->addWatchedService(service);
    m_watchedServices[service].referenceCount += 1;
    if (wasInactive) {
        qCDebug(lcGeoclueMlsdb) << "new watched service, stopping idle timer.";
        m_idleTimer.stop();
    }

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

    if (m_watchedServices.isEmpty()) {
        qCDebug(lcGeoclueMlsdb) << "no watched services, starting idle timer.";
        m_idleTimer.start(QuitIdleTime, this);
    }

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
    tryFetchOnlinePosition();
}

void MlsdbProvider::tryFetchOnlinePosition()
{
    QList<CellPositioningData> cellIds = seenCellIds();

    if (m_onlinePositioningEnabled) {
        if (!m_mlsdbOnlineLocator) {
            m_mlsdbOnlineLocator = new MlsdbOnlineLocator(this);
            connect(m_mlsdbOnlineLocator, &MlsdbOnlineLocator::locationFound,
                    this, &MlsdbProvider::onlineLocationFound);
            connect(m_mlsdbOnlineLocator, &MlsdbOnlineLocator::error,
                    this, &MlsdbProvider::onlineLocationError);
        }
        if (m_mlsdbOnlineLocator->findLocation(cellIds)) {
            return;
        }
    }

    // fall back to using offline position
    updateLocationFromCells(cellIds);
}

void MlsdbProvider::onlineLocationFound(double latitude, double longitude, double accuracy)
{
    qCDebug(lcGeoclueMlsdbPosition) << "Location from MLS online:" << latitude << longitude << accuracy;

    Location deviceLocation;
    deviceLocation.setTimestamp(QDateTime::currentMSecsSinceEpoch());
    deviceLocation.setLatitude(latitude);
    deviceLocation.setLongitude(longitude);

    Accuracy positionAccuracy;
    positionAccuracy.setHorizontal(accuracy);
    deviceLocation.setAccuracy(positionAccuracy);

    setLocation(deviceLocation);
}

void MlsdbProvider::onlineLocationError(const QString &errorString)
{
    qCDebug(lcGeoclueMlsdbPosition) << "Cannot fetch position from online source:" << errorString
                                    << ", falling back to offline source";

    // fall back to using offline position
    updateLocationFromCells(seenCellIds());
}

QList<MlsdbProvider::CellPositioningData> MlsdbProvider::seenCellIds() const
{
    qCDebug(lcGeoclueMlsdbPosition) << "have" << m_cellWatcher->cells().size() << "neighbouring cells";
    QList<CellPositioningData> cells;
    quint32 maxNeighborSignalStrength = 1;
    QSet<MlsdbUniqueCellId> seenCellIds;
    Q_FOREACH (const QSharedPointer<QOfonoExtCell> &c, m_cellWatcher->cells()) {
        CellPositioningData cell;
        quint32 locationCode = 0;
        quint32 cellId = 0;
        quint16 mcc = c->mcc();
        quint16 mnc = c->mnc();
        MlsdbCellType cellType = c->type() == QOfonoExtCell::LTE
                               ? MLSDB_CELL_TYPE_LTE
                               : c->type() == QOfonoExtCell::GSM
                               ? MLSDB_CELL_TYPE_GSM
                               : c->type() == QOfonoExtCell::WCDMA
                               ? MLSDB_CELL_TYPE_UMTS
                               : MLSDB_CELL_TYPE_UMTS;
        if (c->cid() != -1 && c->cid() != 0) {
            locationCode = static_cast<quint32>(c->lac());
            cellId = static_cast<quint32>(c->cid());
        } else if (c->ci() != -1 && c->ci() != 0) {
            locationCode = static_cast<quint32>(c->tac());
            cellId = static_cast<quint32>(c->ci());
        } else {
            qCDebug(lcGeoclueMlsdbPosition) << "ignoring neighbour cell with no cell id with type:" << c->type()
                                            << " mcc:" << c->mcc() << " mnc:" << c->mnc() << " lac:" << c->lac()
                                            << " tac:" << c->tac() << " pci:" << c->pci() << " psc:" << c->psc();
            continue;
        }
        cell.uniqueCellId = MlsdbUniqueCellId(cellType, cellId, locationCode, mcc, mnc);
        if (!seenCellIds.contains(cell.uniqueCellId)) {
            qCDebug(lcGeoclueMlsdbPosition) << "have neighbour cell:" << cell.uniqueCellId.toString()
                                            << "with strength:" << c->signalStrength();
            cell.signalStrength = c->signalStrength();
            if (cell.signalStrength > maxNeighborSignalStrength) {
                // used for the cells we're connected to.
                // if no signal strength data is available from ofono,
                // we assume they're at least as strong signals as the
                // strongest of our neighbor cells.
                maxNeighborSignalStrength = cell.signalStrength;
            }
            cells.append(cell);
            seenCellIds.insert(cell.uniqueCellId);
        }
    }
    return cells;
}

void MlsdbProvider::updateLocationFromCells(const QList<CellPositioningData> &cells)
{
    // determine which cells we have an accurate location for, from MLSDB data.
    double totalSignalStrength = 0.0;
    QMap<MlsdbUniqueCellId, MlsdbCoords> cellLocations;
    Q_FOREACH (const CellPositioningData &cell, cells) {
        MlsdbCoords cellCoords;
        if (!m_uniqueCellIdToLocation.contains(cell.uniqueCellId)) {
            if (m_knownCellIdsWithUnknownLocations.contains(cell.uniqueCellId)) {
                // we know that we don't know the location of this cellId.  Skip it.
                continue;
            } else {
                // this is a new cell Id that we haven't encountered yet.  Probe it.
                if (!searchForCellIdLocation(cell.uniqueCellId, &cellCoords)) {
                    // we now know that we don't know the location of this cellId.
                    m_knownCellIdsWithUnknownLocations.insert(cell.uniqueCellId);
                    continue;
                }
                // cache the location of the cell id for future reference.
                m_uniqueCellIdToLocation.insert(cell.uniqueCellId, cellCoords);
            }
        } else {
            cellCoords = m_uniqueCellIdToLocation.value(cell.uniqueCellId);
        }
        // we have a known location for this cell.  Update our locations list.
        cellLocations.insert(cell.uniqueCellId, cellCoords);
        totalSignalStrength += (1.0 * cell.signalStrength);
    }

    if (cellLocations.size() == 0) {
        qCDebug(lcGeoclueMlsdbPosition) << "no cell id data to calculate position from";
        return;
    } else if (cellLocations.size() == 1) {
        qCDebug(lcGeoclueMlsdbPosition) << "only one cell id datum to calculate position from, position will be extremely inaccurate";
    } else if (cellLocations.size() == 2) {
        qCDebug(lcGeoclueMlsdbPosition) << "only two cell id data to calculate position from, position will be highly inaccurate";
    } else {
        qCDebug(lcGeoclueMlsdbPosition) << "calculating position from" << cellLocations.size() << "cell id data";
    }

    // now use the current cell and neighboringcell information to triangulate our position.
    double deviceLatitude = 0.0;
    double deviceLongitude = 0.0;
    Q_FOREACH (const CellPositioningData &cell, cells) {
        if (cellLocations.contains(cell.uniqueCellId)) {
            const MlsdbCoords &cellCoords(cellLocations.value(cell.uniqueCellId));
            double weight = (((double)cell.signalStrength) / totalSignalStrength);
            deviceLatitude += (weight * cellCoords.lat);
            deviceLongitude += (weight * cellCoords.lon);
            qCDebug(lcGeoclueMlsdbPosition) << "have cell:" << cell.uniqueCellId.toString()
                                            << "with position:" << cellCoords.lat << "," << cellCoords.lon
                                            << "with strength:" << ((double)cell.signalStrength / totalSignalStrength);
        } else {
            qCDebug(lcGeoclueMlsdbPosition) << "do not know position of cell with id:" << cell.uniqueCellId.toString();
        }
    }

    Location deviceLocation;
    if (cellLocations.size()) {
        // estimate accuracy based on how many cells we have.
        Accuracy positionAccuracy;
        positionAccuracy.setHorizontal(qMax(250, (10000 - (1000 * cellLocations.size()))));
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
    if (m_watchedServices.isEmpty()) {
        qCDebug(lcGeoclueMlsdb) << "no watched services, starting idle timer.";
        m_idleTimer.start(QuitIdleTime, this);
    }

    stopPositioningIfNeeded();
}

void MlsdbProvider::updatePositioningEnabled()
{
    bool positioningEnabled = false;
    bool cellPositioningEnabled = false;
    getEnabled(&positioningEnabled, &cellPositioningEnabled, &m_onlinePositioningEnabled);

    bool previous = m_positioningEnabled;
    bool enabled = positioningEnabled && cellPositioningEnabled;
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
    Checks the state of the Location enabled setting,
    the MLS enabled setting, and the MLS online_enabled setting.
*/
void MlsdbProvider::getEnabled(bool *positioningEnabled, bool *cellPositioningEnabled, bool *onlinePositioningEnabled)
{
    QSettings settings(LocationSettingsFile, QSettings::IniFormat);

    *positioningEnabled = settings.value(LocationSettingsEnabledKey, false).toBool();

    *cellPositioningEnabled = *positioningEnabled
                            && (settings.value(LocationSettingsMlsEnabledKey, false).toBool()
                             || settings.value(LocationSettingsOldMlsEnabledKey, false).toBool());

    *onlinePositioningEnabled = *cellPositioningEnabled
                            && settings.value(LocationSettingsMlsOnlineEnabledKey, false).toBool();
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
