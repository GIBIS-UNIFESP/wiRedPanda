// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickUpdateController.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslError>
#include <QStandardPaths>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/Versions.h"

QuickUpdateController::QuickUpdateController(QObject *parent)
    : QObject(parent)
{
    // QNetworkAccessManager::sslErrors doesn't exist at all when Qt is built with QT_NO_SSL --
    // the case for every Qt-for-WebAssembly build (see UpdateController.cpp's identical guard).
#ifndef QT_NO_SSL
    connect(&m_network, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply *reply, const QList<QSslError> &errors) {
        qWarning() << "QuickUpdateController: SSL errors, aborting reply:" << errors;
        reply->abort();
    });
#endif
}

QString QuickUpdateController::currentVersion() const
{
    return QStringLiteral(APP_VERSION);
}

void QuickUpdateController::checkForUpdates()
{
    if (!Application::interactiveMode) {
        return;
    }

    connect(&m_checker, &UpdateChecker::updateAvailable, this, &QuickUpdateController::onChecked);
    m_checker.checkForUpdates();
}

void QuickUpdateController::onChecked(const QString &latestVersion, const QUrl &downloadUrl, const QUrl &releaseUrl)
{
    m_latestVersion = latestVersion;
    m_downloadUrl = downloadUrl;
    m_releaseUrl = releaseUrl;
    emit updateAvailable();
}

void QuickUpdateController::respondToUpdate(bool download, bool skipVersion)
{
    // The check date is recorded by UpdateChecker::onReplyFinished -- the single writer -- so
    // no outcome here needs to touch it, same as the Widgets original.
    if (skipVersion) {
        Settings::setUpdateCheckSkippedVersion(m_latestVersion);
    }

    if (!download) {
        return;
    }

    if (hasDirectDownload()) {
        downloadUpdate();
    } else {
        QDesktopServices::openUrl(m_releaseUrl);
    }
}

void QuickUpdateController::downloadUpdate()
{
    const QString fileName = m_downloadUrl.fileName();
    const QString savePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).filePath(fileName);

    QNetworkRequest request(m_downloadUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(60000);
    m_activeReply = m_network.get(request);

    emit downloadStarted();

    connect(m_activeReply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        if (total > 0) {
            emit downloadProgress(static_cast<int>(received * 100 / total));
        }
    });

    connect(m_activeReply, &QNetworkReply::finished, this, [this, savePath] {
        QNetworkReply *reply = m_activeReply;
        m_activeReply = nullptr;
        reply->deleteLater();
        emit downloadFinished();

        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                Dialogs::provider()->choice(tr("Download Failed"), tr("Could not download the update:\n%1").arg(reply->errorString()),
                    {DialogButton::Ok}, DialogButton::Ok);
            }
            return;
        }

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            Dialogs::provider()->choice(tr("Download Failed"), tr("Could not save the file:\n%1").arg(savePath),
                {DialogButton::Ok}, DialogButton::Ok);
            return;
        }
        const QByteArray payload = reply->readAll();
        if (file.write(payload) != payload.size()) {
            Dialogs::provider()->choice(tr("Download Failed"), tr("Could not write the file:\n%1").arg(savePath),
                {DialogButton::Ok}, DialogButton::Ok);
            return;
        }
        file.close();

        Dialogs::provider()->choice(tr("Download Complete"), tr("wiRedPanda has been downloaded to:\n%1").arg(savePath),
            {DialogButton::Ok}, DialogButton::Ok);
    });
}

void QuickUpdateController::cancelDownload()
{
    if (m_activeReply) {
        m_activeReply->abort();
    }
}
