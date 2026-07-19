// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Checks the wiRedPanda site's published release data for newer versions.
 */

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVersionNumber>

/**
 * \class UpdateChecker
 * \brief Asynchronously queries the site's release data and emits a signal when a newer version is available.
 *
 * \details Call checkForUpdates() once after the main window is shown. The check is skipped if one
 * was already performed today. If the fetched release version is newer than the running application
 * and has not been suppressed by the user, \c updateAvailable is emitted.
 *
 * \note Polls the wiRedPanda site's published \c latest-release.json rather than the GitHub Releases
 * API directly, so the per-user daily check never counts against GitHub's per-IP API rate limit; that
 * file is itself kept fresh by a CI job that queries the API once per release, not once per user.
 */
class UpdateChecker : public QObject
{
    Q_OBJECT

    friend class TestUpdateChecker;

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
    /// The release-data endpoint checkForUpdates() queries (defaults to the site's
    /// published latest-release.json). Overridable only by TestUpdateChecker (via
    /// setApiUrlForTesting()), to point at a local test server instead of the real network.
    QUrl m_apiUrl;

    /// Test-only seam letting TestUpdateChecker redirect checkForUpdates() at
    /// a local QTcpServer instead of the real release-data endpoint.
    void setApiUrlForTesting(const QUrl &url) { m_apiUrl = url; }
};

/// \brief The key in latest-release.json holding the download URL for the given
/// platform ("Windows"/"macOS"/"Linux") and CPU arch token (a QSysInfo arch name,
/// e.g. "x86_64", "arm64"), or an empty string if that combination isn't published.
///
/// \details Exposed for testing. Linux and Windows ship a per-architecture asset, so
/// the arch token selects between two keys; macOS ships a single universal DMG and is
/// keyed on platform alone.
QString releaseAssetKey(const QString &platform, const QString &arch);

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
/// the site's latest-release.json ever populates a download URL with (it's copied verbatim
/// from GitHub's \c browser_download_url; the CDN redirect for the actual asset happens only
/// after fetching this URL, so the field itself never contains the CDN's host). Guards against
/// a compromised/corrupted release-data response smuggling a \c file:// path (silently copies
/// a local file into the Downloads folder) or an unexpected scheme/UNC-style URL into
/// QDesktopServices::openUrl.
bool isSafeGitHubUrl(const QUrl &url);
