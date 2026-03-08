// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/UpdateChecker.h"

#include <QDate>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVersionNumber>

#include "App/Core/Settings.h"
#include "App/Versions.h"

static constexpr auto k_apiUrl = "https://api.github.com/repos/gibis-unifesp/wiredpanda/releases/latest";

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

    // Record the check date only on a successful response so a network error
    // allows a retry on the next application launch.
    Settings::setUpdateCheckLastDate(QDate::currentDate().toString(Qt::ISODate));

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

    const QUrl releaseUrl = QUrl(doc.object().value("html_url").toString());
    emit updateAvailable(latest.toString(), releaseUrl);
}
