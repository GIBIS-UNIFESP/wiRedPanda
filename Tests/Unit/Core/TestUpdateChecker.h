// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestUpdateChecker : public QObject
{
    Q_OBJECT

private slots:

    void init();

    void testUpdateAvailable();
    void testNoUpdate();
    void testReleaseAssetKey();
    void testSafeGitHubUrl();

    void testOnReplyFinishedOffersNewerVersionWithMatchingAsset();
    void testOnReplyFinishedNoMatchingAssetLeavesDownloadUrlEmpty();
    void testOnReplyFinishedNetworkErrorSkipsEverything();
    void testOnReplyFinishedMalformedJsonSkipsEverything();
    void testOnReplyFinishedUpToDateRecordsCheckDateButNoSignal();
    void testOnReplyFinishedUnsafeDownloadUrlFallsBackToReleasePage();

    void testCheckForUpdatesSkippedWhenDisabled();
    void testCheckForUpdatesSkippedWhenAlreadyCheckedToday();
    void testCheckForUpdatesBackpressuresOversizedDownload();
    void testSslErrorsAbortsReply();
};
