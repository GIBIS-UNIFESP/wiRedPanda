// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Checks the GitHub Releases API for newer versions of wiRedPanda.
 */

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>

/**
 * \class UpdateChecker
 * \brief Asynchronously queries the GitHub Releases API and emits a signal when a newer version is available.
 *
 * \details Call checkForUpdates() once after the main window is shown. The check is skipped if one
 * was already performed today. If the fetched release version is newer than the running application
 * and has not been suppressed by the user, \c updateAvailable is emitted.
 */
class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent = nullptr);

    /**
     * \brief Initiates an asynchronous version check.
     *
     * \details Returns immediately. The check is skipped silently if it was already performed today
     * or if the user previously chose to suppress notifications for the latest available version.
     */
    void checkForUpdates();

signals:
    /**
     * \brief Emitted when a newer release is available and has not been suppressed.
     * \param latestVersion Human-readable version string, e.g. "4.5.0".
     * \param releaseUrl    Direct link to the GitHub release page.
     */
    void updateAvailable(const QString &latestVersion, const QUrl &releaseUrl);

private:
    void onReplyFinished(class QNetworkReply *reply);

    QNetworkAccessManager m_network;
};
