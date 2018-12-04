/*
    Copyright (C) 2016 Jolla Ltd.
    Contact: Bea Lam <bea.lam@jollamobile.com>

    This file is part of geoclue-mlsdb.

    Geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#include "mlsdbonlinelocator.h"
#include "mlsdblogging.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>
#include <QtCore/QVariantMap>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QLoggingCategory>
#include <QtGlobal>
#include <QSettings>

#include <sailfishkeyprovider.h>
#include <qofonosimmanager.h>
#include <qofonoextmodemmanager.h>
#include <networkmanager.h>
#include <networkservice.h>

#define REQUEST_REPLY_TIMEOUT_INTERVAL 10000 /* 10 seconds */

#define REQUEST_TIMESTAMPS_TO_TRACK 10
#define REQUEST_BASE_ADAPTIVE_INTERVAL 60000 /* 60 seconds */
#define REQUEST_MODIFY_ADAPTIVE_INTERVAL 10000 /* 10 seconds */

/*
 * HTTP requests are sent based on the Mozilla Location Services API.
 * See https://mozilla.github.io/ichnaea/api/geolocate.html for protocol documentation.
 */

namespace {
    QList<quint32> cellIdsFromQueryData(const QVariantMap &queryData)
    {
        QList<quint32> cellIds;

        const QVariantList cellTowers = queryData.value(QLatin1String("cellTowers")).toList();
        for (const QVariant &ct : cellTowers) {
            const QVariantMap ctm = ct.toMap();
            const quint32 cellId = ctm.value(QLatin1String("cellId")).value<quint32>();
            if (cellId != 0 && !cellIds.contains(cellId)) {
                cellIds.append(cellId);
            }
        }

        return cellIds;
    }
}

MlsdbOnlineLocator::MlsdbOnlineLocator(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_modemManager(new QOfonoExtModemManager(this))
    , m_simManager(0)
    , m_networkManager(new NetworkManager(this))
    , m_currentReply(0)
    , m_fallbacksLacf(true)
    , m_fallbacksIpf(true)
    , m_wlanDataAllowed(true)
    , m_adaptiveInterval(REQUEST_BASE_ADAPTIVE_INTERVAL)
{
    QString MLSConfigFile = QStringLiteral("/etc/gps_xtra.ini");
    QSettings settings(MLSConfigFile, QSettings::IniFormat);
    m_fallbacksLacf = settings.value("MLS/FALLBACKS_LACF", true).toBool();
    m_fallbacksIpf = settings.value("MLS/FALLBACKS_IPF", true).toBool();

    qCDebug(lcGeoclueMlsdb) << "MLS_FALLBACKS_LACF" << m_fallbacksLacf
                            << "MLS_FALLBACKS_IPF" << m_fallbacksIpf;

    connect(m_nam, SIGNAL(finished(QNetworkReply*)), SLOT(requestOnlineLocationFinished(QNetworkReply*)));
    connect(m_modemManager, SIGNAL(enabledModemsChanged(QStringList)), SLOT(enabledModemsChanged(QStringList)));
    connect(m_modemManager, SIGNAL(defaultVoiceModemChanged(QString)), SLOT(defaultVoiceModemChanged(QString)));
    connect(m_networkManager, SIGNAL(servicesChanged()), SLOT(networkServicesChanged()));
    connect(&m_replyTimer, &QTimer::timeout, this, &MlsdbOnlineLocator::timeoutReply);
    m_replyTimer.setInterval(REQUEST_REPLY_TIMEOUT_INTERVAL);
    m_replyTimer.setSingleShot(true);
}

MlsdbOnlineLocator::~MlsdbOnlineLocator()
{
}

void MlsdbOnlineLocator::networkServicesChanged()
{
    if (m_wlanDataAllowed) {
        m_wlanServices = m_networkManager->getServices("wifi");
        emit wlanChanged();
    }
}

void MlsdbOnlineLocator::enabledModemsChanged(const QStringList &modems)
{
    Q_UNUSED(modems);
    setupSimManager();
}

void MlsdbOnlineLocator::defaultVoiceModemChanged(const QString &modem)
{
    Q_UNUSED(modem);
    setupSimManager();
}

bool MlsdbOnlineLocator::wlanDataAllowed() const
{
    return m_wlanDataAllowed;
}

void MlsdbOnlineLocator::setWlanDataAllowed(bool allowed)
{
    if (m_wlanDataAllowed != allowed) {
        m_wlanDataAllowed = allowed;
        emit wlanDataAllowedChanged();
    }
    if (m_wlanDataAllowed && m_wlanServices.isEmpty() && m_networkManager) {
        m_wlanServices = m_networkManager->getServices("wifi");
        emit wlanChanged();
    } else if (!m_wlanDataAllowed) {
        m_wlanServices.clear();
        emit wlanChanged();
    }
}

QPair<QDateTime, QVariantMap> MlsdbOnlineLocator::buildLocationQuery(
        const QList<MlsdbProvider::CellPositioningData> &cells,
        const QPair<QDateTime, QVariantMap> &oldQuery) const
{
    static bool waitForWlanInfo = true;
    const QDateTime currDt = QDateTime::currentDateTimeUtc();
    QVariantMap map;
    map.unite(cellTowerFields(cells));
    map.unite(wlanAccessPointFields());

    if (map.isEmpty()) {
        // no field data(cell, wifi) available
        qCDebug(lcGeoclueMlsdbOnline) << "No field data(cell, wifi) available for MLS online request";
    } else if (!map.contains(QStringLiteral("wifiAccessPoints")) && waitForWlanInfo) {
        // it can take some time to receive wlan network info.
        // the MLS online lookup is far more accurate if we have some wlan network info to provide.
        // so, if we have no wlan info, and this was the first request, don't do an online request yet.
        qCDebug(lcGeoclueMlsdbOnline) << "No wifi data available for MLS online request, postponing";
        waitForWlanInfo = false;
    } else {
        map.unite(globalFields());
        map.unite(fallbackFields());

        // Only send the query if we have more information than previously
        // or if sufficient time has passed since the last query we performed.
        const bool firstTimeQuery = oldQuery.first.isNull() || oldQuery.second.isEmpty();
        const bool intervalExceeded = oldQuery.first.isNull() || oldQuery.first.msecsTo(currDt) >= m_adaptiveInterval;
        const bool moreInfo = map.keys().size() > oldQuery.second.keys().size();
        const bool newCells = cellIdsFromQueryData(oldQuery.second) != cellIdsFromQueryData(map);

        if (firstTimeQuery || intervalExceeded || moreInfo || newCells) {
            // adaptively back-off future requests to avoid server-side throttling.
            static quint32 backOffFactor = 8;

            // we want to aim for approximately 6 minutes per request.
            const qint64 deltaMsecs = (m_queryTimestamps.size() < 3)
                                    ? 0
                                    : (m_queryTimestamps.first() - m_queryTimestamps.last());
            const double minutesPerQuery = (m_queryTimestamps.size() < 3)
                                         ? 6
                                         : ((deltaMsecs / (1000.0 * 60.0)) / m_queryTimestamps.size());
            if (minutesPerQuery > 6 || backOffFactor > 64) {
                // it's been a long time since the last request, reduce the back off factor.
                backOffFactor = backOffFactor <= 2 ? 1 : (backOffFactor / 2);
            } else if (minutesPerQuery < 4) {
                // too many recent requests, increase the back-off factor.
                backOffFactor = backOffFactor >= 32 ? 64 : (backOffFactor * 2);
            }

            // max interval will be about 12 minutes (1 + 10.667 minutes).
            m_adaptiveInterval = REQUEST_BASE_ADAPTIVE_INTERVAL + (REQUEST_MODIFY_ADAPTIVE_INTERVAL * backOffFactor);

            if (backOffFactor == 1 || intervalExceeded) {
                // return the query data for the request.
                qCDebug(lcGeoclueMlsdbOnline) << "Performing MLS online query due to conditions:"
                                              << "first:" << firstTimeQuery
                                              << "interval:" << intervalExceeded
                                              << "info:" << moreInfo
                                              << "cells:" << newCells;
                m_queryTimestamps.prepend(QDateTime::currentMSecsSinceEpoch());
                if (m_queryTimestamps.size() > REQUEST_TIMESTAMPS_TO_TRACK) {
                    m_queryTimestamps.removeLast();
                }
                return qMakePair(currDt, map);
            } else {
                qCDebug(lcGeoclueMlsdbOnline) << "Locally throttling online MLS query due to interval";
            }
        } else {
            qCDebug(lcGeoclueMlsdbOnline) << "No required conditions true for online MLS query!";
        }
    }

    return qMakePair(QDateTime(), QVariantMap());
}

bool MlsdbOnlineLocator::findLocation(const QPair<QDateTime, QVariantMap> &query)
{
    if (query.first.isNull() || query.second.isEmpty()) {
        qCDebug(lcGeoclueMlsdbOnline) << "Empty query data provided";
        return false;
    }

    if (!loadMlsKey()) {
        qCDebug(lcGeoclueMlsdbOnline) << "Unable to load MLS API key";
        return false;
    }

    if (m_currentReply) {
        qCDebug(lcGeoclueMlsdbOnline) << "Previous request still in progress";
        return true;
    }

    QNetworkRequest req(QUrl("https://location.services.mozilla.com/v1/geolocate?key=" + m_mlsKey));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    const QJsonDocument doc = QJsonDocument::fromVariant(query.second);
    const QByteArray json = doc.toJson();
    m_currentReply = m_nam->post(req, json);
    if (m_currentReply->error() != QNetworkReply::NoError) {
        qCDebug(lcGeoclueMlsdbOnline) << "POST request failed:" << m_currentReply->errorString();
        return false;
    }
    m_replyTimer.start();
    qCDebug(lcGeoclueMlsdbOnline) << "Sent request at:" << QDateTime::currentDateTimeUtc().toTime_t() << "with data:" << json;
    return true;
}

void MlsdbOnlineLocator::requestOnlineLocationFinished(QNetworkReply *reply)
{
    if (m_currentReply != reply) {
        qCDebug(lcGeoclueMlsdbOnline) << "Received finished signal for unknown request reply!";
        return;
    }

    QString errorString;
    if (m_currentReply->property("timedOut").toBool()) {
        emit error(QStringLiteral("manual timeout"));
    } else if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray data = m_currentReply->readAll();
        qCDebug(lcGeoclueMlsdbOnline) << "MLS response:" << data;
        if (!readServerResponseData(data, &errorString)) {
            emit error(errorString);
        }
    } else {
        emit error(m_currentReply->errorString());
    }
    m_currentReply->deleteLater();
    m_currentReply = 0;
    m_replyTimer.stop();
}

void MlsdbOnlineLocator::timeoutReply()
{
    qCDebug(lcGeoclueMlsdbOnline) << "Request timed out at:" << QDateTime::currentDateTimeUtc().toTime_t();
    m_currentReply->setProperty("timedOut", QVariant::fromValue<bool>(true));
    m_currentReply->abort(); // will emit finished, the finished slot will deleteLater().
}

bool MlsdbOnlineLocator::readServerResponseData(const QByteArray &data, QString *errorString)
{
    QJsonParseError parseError;
    QJsonDocument json = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        *errorString = parseError.errorString();
        return false;
    }

    if (!json.isObject()) {
        *errorString = "JSON parse error: expected object at root level, not found in " + data;
        return false;
    }

    QJsonObject obj = json.object();
    QVariantMap location = obj.value("location").toVariant().toMap();
    if (location.isEmpty()) {
        *errorString = "JSON parse error: no location data found in " + data;
        return false;
    }

    bool latitudeOk = false;
    bool longitudeOk = false;
    double latitude = location["lat"].toDouble(&latitudeOk);
    double longitude = location["lng"].toDouble(&longitudeOk);
    if (!latitudeOk || !longitudeOk) {
        *errorString = "JSON parse error: latitude or longitude not readable in " + data;
        return false;
    }

    bool accuracyOk = false;
    double accuracy = obj.value("accuracy").toVariant().toDouble(&accuracyOk);
    if (!accuracyOk) {
        accuracy = -1;
    }
    emit locationFound(latitude, longitude, accuracy);
    return true;
}

QVariantMap MlsdbOnlineLocator::globalFields() const
{
    QVariantMap map;
    if (!m_simManager || !m_simManager->isValid()) {
        return map;
    }
    map["carrier"] = m_simManager->serviceProviderName();
    map["considerIp"] = true;
    map["homeMobileCountryCode"] = m_simManager->mobileCountryCode();
    map["homeMobileNetworkCode"] = m_simManager->mobileNetworkCode();
    return map;
}

QVariantMap MlsdbOnlineLocator::cellTowerFields(const QList<MlsdbProvider::CellPositioningData> &cells) const
{
    QVariantMap map;
    if (!cells.isEmpty()) {
        QVariantList cellTowers;
        Q_FOREACH (const MlsdbProvider::CellPositioningData &cell, cells) {
            QVariantMap cellTowerMap;
            // supported radio types: gsm, wcdma or lte
            switch (cell.uniqueCellId.cellType()) {
            case MLSDB_CELL_TYPE_LTE:
                cellTowerMap["radioType"] = "lte";
                break;
            case MLSDB_CELL_TYPE_GSM:
                cellTowerMap["radioType"] = "gsm";
                break;
            case MLSDB_CELL_TYPE_UMTS:
                cellTowerMap["radioType"] = "wcdma";
                break;
            default:
                // type currently unsupported by MLS, don't add it to the field
                break;
            }
            if (cell.uniqueCellId.mcc() != 0) {
                cellTowerMap["mobileCountryCode"] = cell.uniqueCellId.mcc();
            }
            if (cell.uniqueCellId.mnc() != 0) {
                cellTowerMap["mobileNetworkCode"] = cell.uniqueCellId.mnc();
            }
            if (cell.uniqueCellId.locationCode() != 0) {
                cellTowerMap["locationAreaCode"] = cell.uniqueCellId.locationCode();
            }
            if (cell.uniqueCellId.cellId() != 0) {
                cellTowerMap["cellId"] = cell.uniqueCellId.cellId();
            }
            if (cellTowerMap.size() < 5) {
                // "Cell based position estimates require each cell record to contain
                // at least the five radioType, mobileCountryCode, mobileNetworkCode,
                // locationAreaCode and cellId values."
                // https://mozilla.github.io/ichnaea/api/geolocate.html#field-definition
                continue;
            }
            if (cell.signalStrength != 0) {
                // "Position estimates do get a lot more precise if in addition to these
                // unique identifiers at least signalStrength data can be provided for each entry."
                cellTowerMap["signalStrength"] = cell.signalStrength;
            }
            cellTowers.append(cellTowerMap);
        }
        if (!cellTowers.isEmpty()) {
            map["cellTowers"] = cellTowers;
        }
    }

    return map;
}

QVariantMap MlsdbOnlineLocator::wlanAccessPointFields() const
{
    QVariantMap map;
    if (!m_wlanServices.isEmpty()) {
        QVariantList wifiInfoList;
        for (int i = 0; i < m_wlanServices.count(); i++) {
            NetworkService *service = m_wlanServices.at(i);
            if (service->hidden() || service->name().endsWith(QStringLiteral("_nomap"))) {
                // https://mozilla.github.io/ichnaea/api/geolocate.html
                // "Hidden WiFi networks and those whose SSID (clear text name) ends with the string
                // _nomap must NOT be used for privacy reasons."
                continue;
            }
            if (service->bssid().isEmpty()) {
                // "Though in order to get a Bluetooth or WiFi based position estimate at least
                // two networks need to be provided and for each the macAddress needs to be known."
                // https://mozilla.github.io/ichnaea/api/geolocate.html#field-definition
                continue;
            }
            QVariantMap wifiInfo;
            wifiInfo["macAddress"] = service->bssid();
            wifiInfo["frequency"] = service->frequency();
            wifiInfo["signalStrength"] = service->strength();
            wifiInfoList.append(wifiInfo);
        }
        if (wifiInfoList.size() >= 2) {
            // "The minimum of two networks is a mandatory privacy
            // restriction for Bluetooth and WiFi based location services."
            // https://mozilla.github.io/ichnaea/api/geolocate.html#field-definition
            map["wifiAccessPoints"] = wifiInfoList;
        }
    }

    return map;
}

QVariantMap MlsdbOnlineLocator::fallbackFields() const
{
    QVariantMap fallbacks;

    // If no exact cell match can be found, fall back from exact cell position estimates to more
    // coarse grained cell location area estimates, rather than going directly to an even worse
    // GeoIP based estimate.
    fallbacks["lacf"] = m_fallbacksLacf;

    // If no position can be estimated based on any of the provided data points, fall back to an
    // estimate based on a GeoIP database based on the senders IP address at the time of the query.
    fallbacks["ipf"] = m_fallbacksIpf;

    QVariantMap map;
    map["fallbacks"] = fallbacks;
    return map;
}

void MlsdbOnlineLocator::setupSimManager()
{
    if (!m_simManager) {
        m_simManager = new QOfonoSimManager(this);
    }
    // use the default voice modem, or any enabled modem otherwise
    QString modem = m_modemManager->defaultVoiceModem();
    QStringList enabledModems = m_modemManager->enabledModems();
    if (!enabledModems.contains(modem) && !enabledModems.isEmpty()) {
        modem = enabledModems.first();
    }
    if (modem != m_simManager->modemPath()) {
        m_simManager->setModemPath(modem);
    }
}


bool MlsdbOnlineLocator::loadMlsKey()
{
    if (!m_mlsKey.isEmpty()) {
        return true;
    }

    char *keyBuf = NULL;
    int success = SailfishKeyProvider_storedKey("mls", "mls-geolocate", "key", &keyBuf);
    if (keyBuf == NULL) {
        return false;
    } else if (success != 0) {
        free(keyBuf);
        return false;
    }

    m_mlsKey = QLatin1String(keyBuf);
    free(keyBuf);
    return true;
}
