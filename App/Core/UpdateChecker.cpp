// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/UpdateChecker.h"

#include <QDate>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslError>
#include <QVersionNumber>

#include "App/Core/Settings.h"
#include "App/Versions.h"

static constexpr auto k_apiUrl = "https://api.github.com/repos/gibis-unifesp/wiredpanda/releases/latest";

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
{
    connect(&m_network, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply *reply, const QList<QSslError> &errors) {
        qWarning() << "UpdateChecker: SSL errors, aborting reply:" << errors;
        reply->abort();
    });
}

void UpdateChecker::checkForUpdates()
{
    // Skip if we already checked today.
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    if (Settings::updateCheckLastDate() == today) {
        return;
    }

    QNetworkRequest request = QNetworkRequest{QUrl(k_apiUrl)};
    request.setHeader(QNetworkRequest::UserAgentHeader, "wiRedPanda/" APP_VERSION);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(10000);

    QNetworkReply *reply = m_network.get(request);
    // Hard-cap the reply buffer at 1 MiB + 1 byte: when full, Qt stops reading
    // from the socket and TCP backpressure prevents the remote from sending
    // more. The +1 ensures the paired downloadProgress threshold (`> 1 MiB`)
    // can fire before the buffer is exhausted.
    reply->setReadBufferSize(1024 * 1024 + 1);
    // Belt-and-suspenders with setReadBufferSize: setReadBufferSize bounds the
    // buffer; this aborts the reply when the threshold is reached so the
    // connection is torn down rather than left stalled by TCP backpressure.
    connect(reply, &QNetworkReply::downloadProgress, this, [reply](qint64 received, qint64) {
        if (received > 1024 * 1024) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply] { onReplyFinished(reply); });
}

void UpdateChecker::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        // Includes OperationCanceledError when downloadProgress aborts an
        // oversized reply — no further action needed.
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull() || !doc.isObject()) {
        return;
    }

    const QString tagName = doc.object().value("tag_name").toString();
    const QVersionNumber latest = QVersionNumber::fromString(tagName).normalized();
    if (latest.isNull() || latest <= AppVersion::current) {
        return;
    }

    // Respect the user's per-version suppression.
    if (latest.toString() == Settings::updateCheckSkippedVersion()) {
        return;
    }

    QUrl downloadUrl;
    const QJsonArray assets = doc.object().value("assets").toArray();
    for (const QJsonValue &assetVal : assets) {
        const QJsonObject asset = assetVal.toObject();
        const QString name = asset.value("name").toString();
#if defined(Q_OS_WIN)
        const bool match = name.contains("Windows") && name.endsWith(".zip");
#elif defined(Q_OS_MACOS)
        const bool match = name.contains("macOS") && name.endsWith(".dmg");
#elif defined(Q_OS_LINUX)
        const bool match = name.endsWith(".AppImage");
#else
        const bool match = false;
#endif
        if (match) {
            downloadUrl = QUrl(asset.value("browser_download_url").toString());
            break;
        }
    }

    const QUrl releaseUrl = QUrl(doc.object().value("html_url").toString());
    emit updateAvailable(latest.toString(), downloadUrl, releaseUrl);
}
