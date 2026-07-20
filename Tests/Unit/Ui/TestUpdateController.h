// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestUpdateController : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void testCheckForUpdatesSkippedWhenNotInteractive();

    void testShowUpdateDialogDirectDownloadAccepted();
    void testShowUpdateDialogReleasePageFallbackAccepted();
    void testShowUpdateDialogRejectedIsNoOp();
    void testShowUpdateDialogSkipCheckboxPersistsEvenWhenRejected();

    void testDownloadUpdateSuccessSavesFile();
    void testDownloadUpdateNetworkErrorShowsWarning();
    void testDownloadUpdateCanceledSkipsWarning();
    void testDownloadUpdateFileOpenFailureShowsWarning();
};
