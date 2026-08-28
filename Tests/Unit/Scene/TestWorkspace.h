// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestWorkspaceUnit : public QObject
{
    Q_OBJECT

private slots:

    void testWorkspaceCreation();
    void testTabManagement();
    void testModificationTracking();
    void testICEditing();
    void testFullScreenHandling();
    void testMinimapDefaultPositionWithoutPersistedGeometry();
    void testMinimapRestoresPersistedGeometry();
    void testMinimapReclampsOnSubsequentResize();
    void testMinimapRestoreClampsOversizedGeometry();
    void testMinimapIgnoresPreShowResizeThenRestoresOnShow();

    void testResizeRepositionsVisibleExerciseOverlay();
    void testOnMinimapGeometryChangeFinishedPersistsSettings();
    void testSaveWarnsAndNoOpsForNewerVersionFile();
    void testSaveCopiesAssociatedDolphinFile();
    void testSaveRemovesStaleAutosaveFile();
    void testSaveReturnsReadOnlyTargetWhenCommitFailsInteractive();
    void testSaveThrowsWhenCommitFailsNonInteractive();
    void testLoadThrowsWhenFileCannotBeOpened();
    void testLoadMigratesNonPandaSuffixedFileName();
    void testLoadWarnsWhenMigrationTargetIsReadOnly();
    void testLoadWarnsForNewerVersionFile();
    void testLoadInfoForOlderVersionFile();
    void testAutosaveSkipsInlineICTabs();
    void testAutosaveSkipsNewerVersionFile();
    void testAutosaveRemovesFileWhenUndoStackIsClean();
    void testAutosaveFallsBackToAppDataWhenProjectDirIsReadOnly();
    /// An autosave must never be written into the bundled Examples directory: .gitignore
    /// explicitly un-ignores it, so a hidden .<name>.<uuid>.panda beside an opened example
    /// becomes a tracked file. An installed Examples/ is usually read-only and takes the
    /// fallback above; a dev checkout's is writable, so read-only-ness is not the test.
    void testAutosaveFallsBackToAppDataForBundledExamplesDir();
    void testAutosaveRemovesPreviousFileWhenProjectDirChanges();
    void testAutosaveThrowsWhenFileCannotBeOpened();
    void testAutosaveThrowsWhenCommitFails();
};
