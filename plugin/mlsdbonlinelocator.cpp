#include "mlsdbonlinelocator.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>
#include <QtCore/QVariantMap>
#include <QtCore/QTextStream>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QLoggingCategory>

#include <sailfishkeyprovider.h>
#include <qofonosimmanager.h>
#include <qofonoextmodemmanager.h>
#include <networkmanager.h>
#include <networkservice.h>

/*
 * HTTP requests are sent based on the Mozilla Location Services API.
 * See https://mozilla.github.io/ichnaea/api/geolocate.html for protocol documentation.
 */

Q_LOGGING_CATEGORY(lcGeoclueMlsdbOnline, "geoclue.provider.mlsdb.online")

MlsdbOnlineLocator::MlsdbOnlineLocator(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_modemManager(new QOfonoExtModemManager(this))
    , m_simManager(0)
    , m_networkManager(new NetworkManager(this))
{
    connect(m_nam, SIGNAL(finished(QNetworkReply*)), SLOT(requestOnlineLocationFinished(QNetworkReply*)));
    connect(m_modemManager, SIGNAL(enabledModemsChanged(QStringList)), SLOT(enabledModemsChanged(QStringList)));
    connect(m_modemManager, SIGNAL(defaultVoiceModemChanged(QString)), SLOT(defaultVoiceModemChanged(QString)));
    connect(m_networkManager, SIGNAL(servicesChanged()), SLOT(networkServicesChanged()));
}

MlsdbOnlineLocator::~MlsdbOnlineLocator()
{
}

void MlsdbOnlineLocator::networkServicesChanged()
{
    m_wifiServices = m_networkManager->getServices("wifi");
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

bool MlsdbOnlineLocator::findLocation(const QList<MlsdbProvider::CellPositioningData> &cells)
{
    if (!loadMlsKey()) {
        qCDebug(lcGeoclueMlsdbOnline) << "Unable to load MLS API key";
        return false;
    }

    QNetworkRequest req(QUrl("https://location.services.mozilla.com/v1/geolocate?key=" + m_mlsKey));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QVariantMap map;
    map.unite(globalFields());
    map.unite(cellTowerFields(cells));
    map.unite(wifiAccessPointFields());
    map.unite(fallbackFields());

    if (map.isEmpty()) {
        // no field data available
        qCDebug(lcGeoclueMlsdbOnline) << "No field data available for MLS online request";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(map);
    QByteArray json = doc.toJson();
    QNetworkReply *reply = m_nam->post(req, json);
    if (reply->error() != QNetworkReply::NoError) {
        qCDebug(lcGeoclueMlsdbOnline) << "POST request failed:" << reply->errorString();
        return false;
    }
    qCDebug(lcGeoclueMlsdbOnline) << "Sent request with data:" << json;
    return true;
}

void MlsdbOnlineLocator::requestOnlineLocationFinished(QNetworkReply *reply)
{
    QString errorString;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qCDebug(lcGeoclueMlsdbOnline) << "MLS response:" << data;
        if (!readServerResponseData(data, &errorString)) {
            emit error(errorString);
        }
    } else {
        emit error(reply->errorString());
    }
    reply->deleteLater();
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

QVariantMap MlsdbOnlineLocator::globalFields()
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

QVariantMap MlsdbOnlineLocator::cellTowerFields(const QList<MlsdbProvider::CellPositioningData> &cells)
{
    QVariantMap map;
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
            cellTowerMap["wcdma"] = "gsm";
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
        if (cell.signalStrength != 0) {
            cellTowerMap["signalStrength"] = cell.signalStrength;
        }
        cellTowers.append(cellTowerMap);
    }
    map["cellTowers"] = cellTowers;
    return map;
}

QVariantMap MlsdbOnlineLocator::wifiAccessPointFields()
{
    QVariantMap map;
    QVariantList wifiInfoList;
    for (int i=0; i<m_wifiServices.count(); i++) {
        NetworkService *service = m_wifiServices.at(i);
        if (service->hidden()
                || service->name().endsWith(QStringLiteral("_nomap"))) {
            // https://mozilla.github.io/ichnaea/api/geolocate.html
            // "Hidden WiFi networks and those whose SSID (clear text name) ends with the string
            // _nomap must NOT be used for privacy reasons."
            continue;
        }
        QVariantMap wifiInfo;
        wifiInfo["macAddress"] = service->bssid();
        wifiInfo["frequency"] = service->frequency();
        wifiInfo["signalStrength"] = service->strength();
        wifiInfoList.append(wifiInfo);
    }
    map["wifiAccessPoints"] = wifiInfoList;
    return map;
}

QVariantMap MlsdbOnlineLocator::fallbackFields()
{
    QVariantMap fallbacks;

    // If no exact cell match can be found, fall back from exact cell position estimates to more
    // coarse grained cell location area estimates, rather than going directly to an even worse
    // GeoIP based estimate.
    fallbacks["lacf"] = true;

    // If no position can be estimated based on any of the provided data points, fall back to an
    // estimate based on a GeoIP database based on the senders IP address at the time of the query.
    fallbacks["ipf"] = true;

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
