// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickUpdateController: drives the check-for-updates workflow for the Quick chrome.
 */

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

#include "App/Core/UpdateChecker.h"

class QNetworkReply;

/**
 * \class QuickUpdateController
 * \brief Quick-side port of App/UI/UpdateController.h.
 *
 * \details Owns a real UpdateChecker (App/Core, already framework-agnostic -- the network
 * query and version comparison logic needed no porting at all) and adds only the presentation
 * layer UpdateController.cpp hand-rolled with QDialog/QCheckBox/QProgressDialog/QMessageBox:
 * UpdateDialog.qml and UpdateDownloadDialog.qml (wired in Main.qml) replace those, driven by
 * this class's properties/signals. The final download-outcome notice reuses
 * Dialogs::provider()->choice() directly (a plain Ok-only notice, the same pattern every other
 * Quick controller's error/info path already uses) instead of a third bespoke dialog.
 */
class QuickUpdateController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickUpdateController is only ever exposed via AppController.updateController")

    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateAvailable FINAL)
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT FINAL)
    Q_PROPERTY(bool hasDirectDownload READ hasDirectDownload NOTIFY updateAvailable FINAL)

public:
    explicit QuickUpdateController(QObject *parent = nullptr);

    /// Starts an asynchronous version check; emits updateAvailable() if a newer, non-suppressed
    /// release is found. Mirrors UpdateController::checkForUpdates() exactly, including the
    /// interactiveMode gate -- safe to call unconditionally at startup.
    Q_INVOKABLE void checkForUpdates();

    [[nodiscard]] QString latestVersion() const { return m_latestVersion; }
    [[nodiscard]] QString currentVersion() const;
    [[nodiscard]] bool hasDirectDownload() const { return m_downloadUrl.isValid() && !m_downloadUrl.isEmpty(); }

    /// Called once by UpdateDialog.qml when the user dismisses it (Download/Visit-release-page
    /// or Close). Mirrors UpdateController::showUpdateDialog()'s two independent post-dialog
    /// effects: \a skipVersion is honored regardless of \a download, then a direct download or
    /// release-page visit follows only if \a download is true.
    Q_INVOKABLE void respondToUpdate(bool download, bool skipVersion);

    /// Called by UpdateDownloadDialog.qml's Cancel button.
    Q_INVOKABLE void cancelDownload();

signals:
    /// Also acts as the NOTIFY signal for latestVersion/hasDirectDownload -- both are set once,
    /// together, immediately before this fires.
    void updateAvailable();
    void downloadStarted();
    void downloadProgress(int percent);
    /// Emitted once the download attempt is over (success, failure, or cancellation) -- lets
    /// UpdateDownloadDialog.qml close itself; the success/failure notice itself is shown
    /// directly via Dialogs::provider()->choice(), not through this signal.
    void downloadFinished();

private:
    void onChecked(const QString &latestVersion, const QUrl &downloadUrl, const QUrl &releaseUrl);
    void downloadUpdate();

    UpdateChecker m_checker;
    QString m_latestVersion;
    QUrl m_downloadUrl;
    QUrl m_releaseUrl;
    QNetworkAccessManager m_network;
    QNetworkReply *m_activeReply = nullptr;
};
