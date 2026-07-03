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
#include <QSysInfo>
#include <QVersionNumber>

#include "App/Core/Settings.h"
#include "App/Versions.h"

static constexpr auto k_apiUrl = "https://api.github.com/repos/gibis-unifesp/wiredpanda/releases/latest";

/// Compile-time name of the platform this binary was built for, matching the
/// platform token embedded in release asset filenames by deploy.yml.
static QString currentPlatform()
{
#if defined(Q_OS_WIN)
    return "Windows";
#elif defined(Q_OS_MACOS)
    return "macOS";
#elif defined(Q_OS_LINUX)
    return "Linux";
#else
    return {};
#endif
}

bool isMatchingReleaseAsset(const QString &name, const QString &platform, const QString &arch)
{
    if (platform == "Linux") {
        return name.endsWith(".AppImage") && name.contains(arch, Qt::CaseInsensitive);
    }
    if (platform == "Windows") {
        return name.contains("Windows") && name.endsWith(".zip") && name.contains(arch, Qt::CaseInsensitive);
    }
    if (platform == "macOS") {
        // A single universal DMG serves both architectures, so it carries no arch token.
        return name.contains("macOS") && name.endsWith(".dmg");
    }
    return false;
}

bool shouldOfferUpdate(const QString &tagName, const QVersionNumber &currentVersion, const QString &skippedVersion)
{
    const QVersionNumber latest = QVersionNumber::fromString(tagName).normalized();
    if (latest.isNull() || latest <= currentVersion) {
        return false;
    }

    // Respect the user's per-version suppression.
    return latest.toString() != skippedVersion;
}

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
    // The GitHub API response is realistically tens of KB; cap well above that
    // as defense-in-depth against a hostile/corrupted endpoint buffering unbounded
    // bytes into memory before onReplyFinished ever gets a chance to react.
    reply->setReadBufferSize(1024 * 1024 + 1);
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

    // Also covers QNetworkReply::OperationCanceledError produced by the
    // read-buffer size-cap abort in checkForUpdates().
    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull() || !doc.isObject()) {
        return;
    }

    // A successful, parseable reply IS the daily check — record it here, not
    // when a dialog is shown: with no newer release (the common case) the
    // date was never written and the API was hit on every launch. Network
    // failures above intentionally don't record, so the check retries.
    Settings::setUpdateCheckLastDate(QDate::currentDate().toString(Qt::ISODate));

    const QString tagName = doc.object().value("tag_name").toString();
    if (!shouldOfferUpdate(tagName, AppVersion::current, Settings::updateCheckSkippedVersion())) {
        return;
    }
    const QVersionNumber latest = QVersionNumber::fromString(tagName).normalized();

    QUrl downloadUrl;
    const QString platform = currentPlatform();
    const QString arch = QSysInfo::buildCpuArchitecture(); // "x86_64" / "arm64"
    const QJsonArray assets = doc.object().value("assets").toArray();
    for (const QJsonValue &assetVal : assets) {
        const QJsonObject asset = assetVal.toObject();
        const QString name = asset.value("name").toString();
        if (isMatchingReleaseAsset(name, platform, arch)) {
            downloadUrl = QUrl(asset.value("browser_download_url").toString());
            break;
        }
    }

    const QUrl releaseUrl = QUrl(doc.object().value("html_url").toString());
    emit updateAvailable(latest.toString(), downloadUrl, releaseUrl);
}
