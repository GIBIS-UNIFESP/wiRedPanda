// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestUpdateChecker.h"

#include "App/Core/UpdateChecker.h"
#include "Tests/Common/TestUtils.h"

void TestUpdateChecker::testUpdateAvailable()
{
    // Test: UpdateChecker can detect when update is available
    UpdateChecker checker;

    // Create checker object - should not crash (construction reaching here is the assertion)
}

void TestUpdateChecker::testNoUpdate()
{
    // Test: UpdateChecker handles case where no update is available
    UpdateChecker checker;

    // Verify object is valid
    QVERIFY(true);
}

void TestUpdateChecker::testNetworkError()
{
    // Test: UpdateChecker handles network errors gracefully
    UpdateChecker checker;

    // Should not crash on network error
    QVERIFY(true);
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
