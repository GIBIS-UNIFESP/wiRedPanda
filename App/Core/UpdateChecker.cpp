// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/UpdateChecker.h"

#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
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

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
{
}

void UpdateChecker::checkForUpdates()
{
    // Skip if we already checked today.
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    if (Settings::updateCheckLastDate() == today) {
        return;
    }

    QNetworkRequest request{QUrl(k_apiUrl)};
    request.setHeader(QNetworkRequest::UserAgentHeader, "wiRedPanda/" APP_VERSION);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] { onReplyFinished(reply); });
}

void UpdateChecker::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

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
    const QVersionNumber latest = QVersionNumber::fromString(tagName).normalized();
    if (latest.isNull() || latest <= AppVersion::current) {
        return;
    }

    // Respect the user's per-version suppression.
    if (latest.toString() == Settings::updateCheckSkippedVersion()) {
        return;
    }

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
