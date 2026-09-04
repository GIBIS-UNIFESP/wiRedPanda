// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickUpdateController: CanvasItem-free port of App/UI/UpdateController.h.
 */

#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

class QNetworkReply;

/**
 * \class QuickUpdateController
 * \brief Owns the update-check lifecycle for wiredpanda: querying for a newer release
 * and driving the QML-side notification/download UI.
 *
 * \details Wraps the same, already-portable App/Core/UpdateChecker.h unchanged, but exposes
 * plain signals (`updateAvailable`/`downloadProgress`/`downloadFinished`) that a QML dialog
 * (UpdateDialog.qml) binds to instead of UpdateController's QDialog/QProgressDialog/
 * QMessageBox-driven `showUpdateDialog()`/`downloadUpdate()` bodies. This class owns no dialog
 * of its own, unlike the DialogProvider-routed confirm/prompt dialogs elsewhere in this chrome:
 * a checkbox plus conditional Download/Close buttons plus a progress bar don't fit that
 * abstraction's generic choice()/textPrompt() shapes (ExerciseOverlay.qml/TourOverlay.qml use
 * the same reasoning for their own bespoke overlays).
 *
 * `isSafeGitHubUrl()` validation happens inside UpdateChecker::onReplyFinished() before
 * `updateAvailable` is ever emitted, so neither this class nor its QML dialog need to
 * re-validate either URL.
 */
class QuickUpdateController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by QuickAppController")

    // FINAL: never subclassed, matching every other presenter in this Quick chrome.
    Q_PROPERTY(bool autoCheckEnabled READ isAutoCheckEnabled WRITE setAutoCheckEnabled NOTIFY autoCheckEnabledChanged FINAL)

public:
    explicit QuickUpdateController(QObject *parent = nullptr);

    [[nodiscard]] bool isAutoCheckEnabled() const;
    void setAutoCheckEnabled(bool enabled);

    /// Starts an asynchronous version check; a no-op (and skipped silently) outside interactive
    /// mode, if checked already today, or if auto-checks are disabled -- all three guards live
    /// in UpdateChecker::checkForUpdates()/Settings, unchanged. Mirrors
    /// UpdateController::checkForUpdates(); call once after the window is shown.
    Q_INVOKABLE void checkForUpdates();

    /// Records \a version as suppressed -- the "Don't notify me about this version again"
    /// checkbox. Mirrors UpdateController::showUpdateDialog()'s skip-checkbox handling.
    Q_INVOKABLE void skipVersion(const QString &version);

    /// Opens \a url in the system browser. Mirrors showUpdateDialog()'s no-direct-download
    /// fallback (QDesktopServices::openUrl(releaseUrl)).
    Q_INVOKABLE void openReleasePage(const QUrl &url);

    /// Downloads \a url to the user's Downloads directory, emitting downloadProgress() as it
    /// goes and downloadFinished() once done (success or failure). A no-op if a download is
    /// already in flight -- mirrors downloadUpdate()'s "raise the existing dialog instead of
    /// starting a second one" guard, without a dialog of its own to raise.
    Q_INVOKABLE void downloadUpdate(const QString &latestVersion, const QUrl &url);

    /// Aborts the in-flight download, if any. Mirrors the progress dialog's Cancel button.
    Q_INVOKABLE void cancelDownload();

signals:
    void autoCheckEnabledChanged();

    /// Emitted when a newer release is available and has not been suppressed -- relayed
    /// directly from UpdateChecker::updateAvailable(). \a latestVersion is human-readable
    /// (e.g. "4.5.0"); \a downloadUrl is the platform binary asset, or empty if none matched;
    /// \a releaseUrl is the GitHub release page, used as the fallback / "view release" link.
    void updateAvailable(const QString &latestVersion, const QUrl &downloadUrl, const QUrl &releaseUrl);

    /// \a percent is 0-100. Only emitted while the response's Content-Length is known.
    void downloadProgress(int percent);

    /// \a success is false for both a real failure and a user-initiated cancelDownload(); the
    /// canonical way to tell them apart from the QML side is that a cancellation ends the
    /// download without ever emitting this signal at all -- mirrors downloadUpdate()'s own
    /// "OperationCanceledError is not an error worth telling the user about" distinction.
    /// \a message is a user-facing success/failure string, ready to show as-is.
    void downloadFinished(bool success, const QString &message);

private:
    QPointer<QNetworkReply> m_activeReply;
};
