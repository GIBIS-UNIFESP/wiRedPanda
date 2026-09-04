// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Feature-gap decision 8 (post-hoc, 2026-09-01): QuickUpdateController had zero test coverage
/// (App/QuickShell had zero Update* presence at all before this class existed). Original test
/// authoring for the signal-relay/download-flow surface this class adds on top of the
/// already-covered, unchanged App/Core/UpdateChecker.h (see Tests/Unit/Core/TestUpdateChecker.cpp
/// for that coverage, reused as-is). Not a full port of Tests/Unit/Ui/TestUpdateController.cpp's
/// dialog-driven tests -- this class owns no dialog of its own (see its class doc comment), so
/// there is no QDialog/QProgressDialog/QMessageBox for those tests' techniques to apply to;
/// downloadUpdate()'s actual network/file-write logic is a verbatim copy of
/// UpdateController::downloadUpdate()'s already-tested body, so only its success/cancel/no-op
/// paths are re-verified here, via the same local-HTTP-server technique
/// TestUpdateController.cpp/TestUpdateChecker.cpp both already use.
class TestQuickUpdateController : public QObject
{
    Q_OBJECT

private slots:
    void testAutoCheckEnabledDefaultsToTrueAndRoundTrips();

    void testSkipVersionPersists();

    void testCheckForUpdatesSkippedWhenNotInteractive();

    void testDownloadUpdateSuccessEmitsProgressAndFinished();
    void testDownloadUpdateNoOpWhileAlreadyInFlight();
    void testCancelDownloadAbortsWithoutFinishedSignal();
};
