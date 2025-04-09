/*
    Copyright (C) 2016 Jolla Ltd.
    Contact: Bea Lam <bea.lam@jollamobile.com>

    This file is part of geoclue-mlsdb.

    Geoclue-mlsdb is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License.
*/

#ifndef MLSDBONLINELOCATOR_H
#define MLSDBONLINELOCATOR_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QTimer>

#include <MDConfItem>

#include "mlsdbprovider.h"

QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager)
QT_FORWARD_DECLARE_CLASS(QNetworkReply)
class QOfonoExtModemManager;
class QOfonoSimManager;
class NetworkManager;
class NetworkService;

/*
 * The MlsdbOnlineLocator class looks up the current location from the
 * online Mozilla Location Service database.
 */

class MlsdbOnlineLocator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool wlanDataAllowed READ wlanDataAllowed WRITE setWlanDataAllowed NOTIFY wlanDataAllowedChanged)

public:
    explicit MlsdbOnlineLocator(QObject *parent = 0);
    ~MlsdbOnlineLocator();

    bool wlanDataAllowed() const;
    void setWlanDataAllowed(bool allowed);

    QPair<QDateTime, QVariantMap> buildLocationQuery(
        const QList<MlsdbProvider::CellPositioningData> &cells,
        const QPair<QDateTime, QVariantMap> &oldQuery) const;
    bool findLocation(const QPair<QDateTime, QVariantMap> &request);

signals:
    void locationFound(double latitude, double longitude, double accuracy);
    void error(const QString &errorString);
    void wlanChanged();
    void wlanDataAllowedChanged();

private Q_SLOTS:
    void networkServicesChanged();
    void enabledModemsChanged(const QStringList &modems);
    void defaultVoiceModemChanged(const QString &modem);
    void requestOnlineLocationFinished(QNetworkReply *reply);
    void timeoutReply();

private:
    bool readServerResponseData(const QByteArray &data, QString *errorString);
    void checkError(const QByteArray &data);

    QVariantMap globalFields() const;
    QVariantMap cellTowerFields(const QList<MlsdbProvider::CellPositioningData> &cells) const;
    QVariantMap wlanAccessPointFields() const;
    QVariantMap fallbackFields() const;

    void setupSimManager();
    bool loadMlsKey();

    QNetworkAccessManager *m_nam;
    QOfonoExtModemManager *m_modemManager;
    QOfonoSimManager *m_simManager;
    NetworkManager *m_networkManager;
    QNetworkReply *m_currentReply;
    QTimer m_replyTimer;

    QVector<NetworkService*> m_wlanServices;
    QString m_mlsKey;
    QString m_mlsUrl;

    bool m_fallbacksLacf;
    bool m_fallbacksIpf;

    bool m_wlanDataAllowed;

    mutable quint32 m_adaptiveInterval;
    mutable QVector<qint64> m_queryTimestamps;

    MDConfItem m_keyFailureTime;
};

#endif // MLSDBONLINELOCATOR_H
