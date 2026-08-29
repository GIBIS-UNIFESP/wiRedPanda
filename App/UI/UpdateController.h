// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief UpdateController: drives the application's check-for-updates workflow.
 */

#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>

class QProgressDialog;
class QWidget;

/**
 * \class UpdateController
 * \brief Owns the update-check lifecycle: querying for a newer release and
 * presenting the resulting notification / download dialogs.
 *
 * \details Extracted from MainWindow. Couples only to a parent QWidget (for
 * dialog parenting) and Settings; it has no dependency on tabs, scenes, or
 * circuit state. Call checkForUpdates() once after the window is shown.
 */
class UpdateController : public QObject
{
    Q_OBJECT

    friend class TestUpdateController;

public:
    /// \param parent Widget used to parent the modal update dialogs (and as QObject parent).
    explicit UpdateController(QWidget *parent);

    /// Starts an asynchronous version check; shows the update dialog if one is available.
    void checkForUpdates();

private:
    /// Presents the "update available" dialog and dispatches download / release-page actions.
    void showUpdateDialog(const QString &latestVersion, const QUrl &downloadUrl, const QUrl &releaseUrl);

    /// Downloads \a url to the user's Downloads directory with a progress dialog.
    void downloadUpdate(const QString &latestVersion, const QUrl &url);

    QWidget *m_parent = nullptr;

    /// The in-flight download's progress dialog, if any. Tracked because that dialog is no
    /// longer window-modal (see downloadUpdate), so the main window stays interactive and a
    /// second Check for Updates could otherwise start a concurrent download writing the same
    /// file. QPointer: the dialog deletes itself when the download ends.
    QPointer<QProgressDialog> m_progressDialog;
};
