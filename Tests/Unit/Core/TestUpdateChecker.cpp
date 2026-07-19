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

void TestUpdateChecker::testReleaseAssetKey()
{
    // Each platform/arch combination must select its own key in latest-release.json.
    QCOMPARE(releaseAssetKey("Linux", "x86_64"), QStringLiteral("linuxX64"));
    QCOMPARE(releaseAssetKey("Linux", "arm64"), QStringLiteral("linuxArm64"));
    QCOMPARE(releaseAssetKey("Windows", "x86_64"), QStringLiteral("windowsX64"));
    QCOMPARE(releaseAssetKey("Windows", "arm64"), QStringLiteral("windowsArm64"));

    // macOS: single universal DMG, keyed on platform regardless of arch token.
    QCOMPARE(releaseAssetKey("macOS", "x86_64"), QStringLiteral("macosUniversal"));
    QCOMPARE(releaseAssetKey("macOS", "arm64"), QStringLiteral("macosUniversal"));

    // Unknown architecture resolves to no key where one is required — the caller
    // then falls back to the release page rather than a wrong binary.
    QVERIFY(releaseAssetKey("Linux", "ppc64").isEmpty());
    QVERIFY(releaseAssetKey("Windows", "ppc64").isEmpty());

    // Unknown platform resolves to no key at all.
    QVERIFY(releaseAssetKey("BeOS", "x86_64").isEmpty());
}

void TestUpdateChecker::testSafeGitHubUrl()
{
    // Regression: a download URL from the (response-controlled) release data was used for a
    // network download + file write with no scheme/host check. The site's latest-release.json
    // only ever populates that field as https://github.com/... — the CDN redirect happens only
    // after fetching this URL, so its host never appears here.
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
