// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickUpdateController.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslError>
#include <QStandardPaths>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"
#include "App/Core/UpdateChecker.h"

QuickUpdateController::QuickUpdateController(QObject *parent)
    : QObject(parent)
{
}

bool QuickUpdateController::isAutoCheckEnabled() const
{
    return !Settings::updateChecksDisabled();
}

void QuickUpdateController::setAutoCheckEnabled(bool enabled)
{
    Settings::setUpdateChecksDisabled(!enabled);
    emit autoCheckEnabledChanged();
}

void QuickUpdateController::checkForUpdates()
{
    if (!Application::interactiveMode) {
        return;
    }

    auto *checker = new UpdateChecker(this);
    connect(checker, &UpdateChecker::updateAvailable, this, &QuickUpdateController::updateAvailable);
    checker->checkForUpdates();
}

void QuickUpdateController::skipVersion(const QString &version)
{
    Settings::setUpdateCheckSkippedVersion(version);
}

void QuickUpdateController::openReleasePage(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

void QuickUpdateController::downloadUpdate(const QString &latestVersion, const QUrl &url)
{
    Q_UNUSED(latestVersion)

    // No dialog of our own to raise (see class doc comment) -- a no-op is the correct guard
    // here, same as UpdateController::downloadUpdate()'s "raise the existing one" branch.
    if (m_activeReply) {
        return;
    }

    const QString fileName = url.fileName();
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    // Not guaranteed to already exist -- e.g. a minimal/headless environment with no
    // pre-populated XDG user-dirs -- and QFile::open(WriteOnly) doesn't create missing parent
    // directories.
    QDir().mkpath(downloadDir);
    const QString savePath = QDir(downloadDir).filePath(fileName);

    auto *network = new QNetworkAccessManager(this);
    // QNetworkAccessManager::sslErrors doesn't exist at all when Qt is built with QT_NO_SSL --
    // the case for every Qt-for-WebAssembly build, since WASM has no native TLS backend and
    // routes network access through the browser instead.
#ifndef QT_NO_SSL
    connect(network, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply *reply, const QList<QSslError> &errors) {
        qWarning() << "QuickUpdateController::downloadUpdate: SSL errors, aborting reply:" << errors;
        reply->abort();
    });
#endif

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(60000);
    QNetworkReply *reply = network->get(request);
    m_activeReply = reply;

    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        if (total > 0) {
            emit downloadProgress(static_cast<int>(received * 100 / total));
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, savePath] {
        if (m_activeReply == reply) {
            m_activeReply.clear();
        }

        if (reply->error() != QNetworkReply::NoError) {
            // A user-initiated cancelDownload() ends here silently, same as
            // UpdateController::downloadUpdate()'s identical guard -- not a failure worth
            // telling the user about.
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                emit downloadFinished(false, tr("Could not download the update:\n%1").arg(reply->errorString()));
            }
            reply->deleteLater();
            return;
        }

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            emit downloadFinished(false, tr("Could not save the file:\n%1").arg(savePath));
            reply->deleteLater();
            return;
        }
        const QByteArray payload = reply->readAll();
        if (file.write(payload) != payload.size()) {
            emit downloadFinished(false, tr("Could not write the file:\n%1").arg(savePath));
            reply->deleteLater();
            return;
        }
        file.close();
        reply->deleteLater();

        emit downloadFinished(true, tr("wiRedPanda has been downloaded to:\n%1").arg(savePath));
    });
}

void QuickUpdateController::cancelDownload()
{
    // Known, recurring GCC 15 -Wnull-dereference false positive under an optimized preset
    // (-O2/-O3): fires on QPointer::operator->()'s own inlined internals even with the
    // preceding truthiness check -- see project memory
    // project_release_null_deref_quickappcontroller.md ("a call-site null-guard does NOT
    // silence it").
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#endif
    if (m_activeReply) {
        m_activeReply->abort();
    }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}
