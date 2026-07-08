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
#include <QVersionNumber>

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
     * \param downloadUrl   Direct URL to the platform binary asset, or empty if none matched.
     * \param releaseUrl    Fallback link to the GitHub release page.
     */
    void updateAvailable(const QString &latestVersion, const QUrl &downloadUrl, const QUrl &releaseUrl);

private:
    void onReplyFinished(class QNetworkReply *reply);

    QNetworkAccessManager m_network;
};

/// \brief True when a GitHub release asset filename is the binary for the given
/// platform ("Windows"/"macOS"/"Linux") and CPU arch token (a QSysInfo arch name,
/// e.g. "x86_64", "arm64").
///
/// \details Exposed for testing. Linux (.AppImage) and Windows (.zip) releases
/// ship a per-architecture asset, so the arch token must also match; macOS ships
/// a single universal DMG and is matched on platform alone.
bool isMatchingReleaseAsset(const QString &name, const QString &platform, const QString &arch);

/// \brief True when the release tagged \p tagName should be offered to a user
/// running \p currentVersion who may have suppressed \p skippedVersion.
///
/// \details Exposed for testing. A release is offered only when its tag parses
/// to a valid version that is strictly newer than the running application and
/// whose normalized string does not match the user's per-version suppression.
bool shouldOfferUpdate(const QString &tagName, const QVersionNumber &currentVersion, const QString &skippedVersion);

/// \brief True when \a url is safe to download from or hand to the OS's URL handler.
///
/// \details Exposed for testing. Requires scheme https and host github.com — the only shape
/// GitHub's release JSON ever populates \c browser_download_url / \c html_url with (the CDN
/// redirect for the actual asset happens only after fetching this URL, so the field itself
/// never contains the CDN's host). Guards against a compromised/future-relaxed API response
/// smuggling a \c file:// path (silently copies a local file into the Downloads folder) or an
/// unexpected scheme/UNC-style URL into QDesktopServices::openUrl.
bool isSafeGitHubUrl(const QUrl &url);
