// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestUpdateChecker.h"

#include <QUrl>

#include "App/Core/UpdateChecker.h"
#include "Tests/Common/TestUtils.h"

void TestUpdateChecker::testUpdateAvailable()
{
    // shouldOfferUpdate must offer any release strictly newer than the running
    // version, unless the user suppressed exactly that version.
    const QVersionNumber current(5, 1, 2);

    QVERIFY(shouldOfferUpdate("5.1.3", current, {}));   // newer patch
    QVERIFY(shouldOfferUpdate("5.2", current, {}));     // newer minor
    QVERIFY(shouldOfferUpdate("6.0", current, {}));     // newer major
    QVERIFY(shouldOfferUpdate("5.1.3.1", current, {})); // extra segment, still newer

    // Suppressing a DIFFERENT version must not swallow the new release.
    QVERIFY(shouldOfferUpdate("5.1.3", current, "5.1.1"));
    QVERIFY(shouldOfferUpdate("5.2", current, "5.1.3"));
}

void TestUpdateChecker::testNoUpdate()
{
    const QVersionNumber current(5, 1, 2);

    // Same or older releases are never offered.
    QVERIFY(!shouldOfferUpdate("5.1.2", current, {}));
    QVERIFY(!shouldOfferUpdate("5.1.1", current, {}));
    QVERIFY(!shouldOfferUpdate("5.0", current, {}));
    QVERIFY(!shouldOfferUpdate("4.9.9", current, {}));

    // Trailing zeros normalize away: "5.1.2.0" is the running version.
    QVERIFY(!shouldOfferUpdate("5.1.2.0", current, {}));

    // Malformed or empty tags parse to a null version — never offered.
    QVERIFY(!shouldOfferUpdate({}, current, {}));
    QVERIFY(!shouldOfferUpdate("not-a-version", current, {}));

    // Per-version suppression: the suppressed release itself stays silent.
    // (The suppression is matched against the NORMALIZED version string.)
    QVERIFY(!shouldOfferUpdate("5.1.3", current, "5.1.3"));
    QVERIFY(!shouldOfferUpdate("5.2.0", current, "5.2"));
}

void TestUpdateChecker::testAssetSelection()
{
    // Regression: a release ships per-architecture Linux AppImages and Windows
    // ZIPs. The GitHub API returns assets sorted alphabetically, so "arm64"
    // precedes "x86_64"; selecting by suffix alone wrongly picked the arm64
    // binary for x86_64 users. Pin architecture-aware selection here. These are
    // the real 5.1.2 asset names.
    const QString linuxArm = "wiRedPanda-5.1.2-Linux-arm64-Qt6.9.3.AppImage";
    const QString linuxX86 = "wiRedPanda-5.1.2-Linux-x86_64-Qt6.9.3.AppImage";
    const QString macUniversal = "wiRedPanda-5.1.2-macOS-Qt6.9.3.dmg";
    const QString winArm = "wiRedPanda-5.1.2-Windows-arm64-Qt6.9.3-Portable.zip";
    const QString winX86 = "wiRedPanda-5.1.2-Windows-x86_64-Qt6.9.3-Portable.zip";

    // Linux: each arch matches only its own AppImage (the exact reported bug).
    QVERIFY(isMatchingReleaseAsset(linuxX86, "Linux", "x86_64"));
    QVERIFY(!isMatchingReleaseAsset(linuxArm, "Linux", "x86_64"));
    QVERIFY(isMatchingReleaseAsset(linuxArm, "Linux", "arm64"));
    QVERIFY(!isMatchingReleaseAsset(linuxX86, "Linux", "arm64"));

    // Windows: same latent defect, same guard.
    QVERIFY(isMatchingReleaseAsset(winX86, "Windows", "x86_64"));
    QVERIFY(!isMatchingReleaseAsset(winArm, "Windows", "x86_64"));
    QVERIFY(isMatchingReleaseAsset(winArm, "Windows", "arm64"));
    QVERIFY(!isMatchingReleaseAsset(winX86, "Windows", "arm64"));

    // A Linux build must never select a Windows or macOS asset, and vice-versa.
    QVERIFY(!isMatchingReleaseAsset(winX86, "Linux", "x86_64"));
    QVERIFY(!isMatchingReleaseAsset(linuxX86, "Windows", "x86_64"));

    // macOS: single universal DMG, matched on platform regardless of arch token.
    QVERIFY(isMatchingReleaseAsset(macUniversal, "macOS", "x86_64"));
    QVERIFY(isMatchingReleaseAsset(macUniversal, "macOS", "arm64"));
    QVERIFY(!isMatchingReleaseAsset(linuxX86, "macOS", "x86_64"));

    // Unknown architecture matches nothing where an arch token is required —
    // the caller then falls back to the release page rather than a wrong binary.
    QVERIFY(!isMatchingReleaseAsset(linuxX86, "Linux", "ppc64"));
    QVERIFY(!isMatchingReleaseAsset(winX86, "Windows", "ppc64"));
}

void TestUpdateChecker::testSafeGitHubUrl()
{
    // Regression: browser_download_url/html_url from the GitHub API response were used for a
    // network download + file write and QDesktopServices::openUrl with no scheme/host check.
    // GitHub's release JSON only ever populates these fields as https://github.com/... — the
    // CDN redirect happens only after fetching this URL, so its host never appears here.
    QVERIFY(isSafeGitHubUrl(QUrl("https://github.com/gibis-unifesp/wiredpanda/releases/download/5.1.2/wiRedPanda.AppImage")));
    QVERIFY(isSafeGitHubUrl(QUrl("https://github.com/gibis-unifesp/wiredpanda/releases/tag/5.1.2")));

    // Wrong scheme: could otherwise have the app silently copy a local file into Downloads,
    // or hand an unexpected URI scheme to the OS's protocol handler.
    QVERIFY(!isSafeGitHubUrl(QUrl("file:///etc/passwd")));
    QVERIFY(!isSafeGitHubUrl(QUrl("ftp://github.com/some/asset")));

    // Lookalike hosts must not pass a substring/prefix check.
    QVERIFY(!isSafeGitHubUrl(QUrl("https://github.com.evil.com/asset")));
    QVERIFY(!isSafeGitHubUrl(QUrl("https://evil.com/github.com")));

    // Invalid/empty URL.
    QVERIFY(!isSafeGitHubUrl(QUrl()));
    QVERIFY(!isSafeGitHubUrl(QUrl(QString())));
}
