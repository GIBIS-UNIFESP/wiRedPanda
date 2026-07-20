// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestWorkspaceManager : public QObject
{
    Q_OBJECT

private slots:

    void testAccessorsAreSafeWithNoCurrentTab();
    void testDolphinFileNameRoundTripsThroughCurrentTab();

    void testSaveWithNoExistingFileAndCancelledDialogIsNoOp();
    void testSaveReadOnlyRetryCancelledReturnsEarly();
    void testSaveRemovesAutosaveRecordOnSuccess();

    void testCloseTabCancelledConfirmationReturnsFalse();
    void testCloseTabDeclinedConfirmationClosesWithoutSaving();
    void testCloseTabSaveFailureThenCloseAnywayDiscards();
    void testCloseTabSaveFailureThenCloseAnywayKeepsOpen();

    void testConfirmSaveSingleOffersBasicButtons();
    void testConfirmSaveMultipleOffersAllButtons();

    void testOpenICInTabIsNoOpWithNoCurrentTab();

    void testSaveFileCancelledDialogIsNoOp();
    void testSaveFileAppendsExtensionWhenDialogOmitsIt();
    void testSaveFileWarnsWhenTargetOpenInAnotherTab();
    void testSaveFileAsCancelledIsNoOp();

    void testReloadFileIsNoOpWithoutAnExistingFile();

    void testCloseFilesClosesEveryTab();
    void testCloseFilesStopsWhenATabCancels();

    void testLoadAutosaveFilesRemovesMissingFile();
    void testLoadAutosaveFilesRemovesCorruptedFile();

    void testSetCurrentFileFallsBackToCurrentTabWhenNotCalledAsSlot();
    void testSetCurrentFileIsNoOpWithNoSenderAndNoCurrentTab();
    void testSetCurrentFileIsNoOpWhenSenderTabNotInTabWidget();
};
