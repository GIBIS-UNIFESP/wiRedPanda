// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Tests QuickWorkSpace (App/QuickShell/Chrome/QuickWorkSpace.h/.cpp): autosave triggering,
/// naming, and cleanup; save/load round-trips; external-dependency copying on save; file-state
/// management; and format migration.
class TestQuickWorkSpace : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    // --- Autosave triggering ---
    void testAutosaveTriggersOnCircuitChange();
    void testAutosaveSkippedWhenClean();
    void testAutosaveAfterElementModify();
    void testAutosaveAfterElementDelete();
    void testAutosaveSignalEmitted();

    // --- Autosave file naming ---
    void testAutosaveFileCreatedInAppDataForNewProject();
    void testAutosaveInCurrentDirForSavedProject();
    void testAutosaveFileNamingPatternIsLoadable();
    void testAutosavePathCreatedIfNotExists();
    void testAutosaveFilePermissions();
    void testMultipleWorkspacesProduceDistinctAutosaves();

    // --- Autosave cleanup ---
    void testAutosaveDeletedOnExplicitSave();
    void testFlushPendingAutosaveRunsImmediatelyB3();
    void testAutosaveTruncatesOnShrinkB2();

    // --- Save operations ---
    void testSaveToNewFile();
    void testSaveToExistingFileOverwrites();
    void testSaveWithSpecialCharactersInFilename();
    void testSavePreservesElementData();
    void testSaveEmptyCircuit();

    // --- External-file dependency copying on save ---
    void testSaveCopiesExternalAppearanceFileOnFirstSave();
    void testSaveCopiesNewlyAddedDependencyOnResave();

    // --- Load operations ---
    void testLoadNonExistentFileThrowsAndLeavesProjectUnchanged();
    void testLoadCorruptedFileHandling();
    void testLoadEmptyCircuit();
    void testSaveToInvalidPathThrows();

    // --- File state management ---
    void testModifiedFlagAfterAddElement();
    void testModifiedFlagClearedAfterSave();
    void testFilePathUpdatedAfterSave();
    void testFilePathPreservedAfterLoad();
    void testSaveFailureLeavesFileIdentityUnchanged();

    // --- Metadata / lastId ---
    void testDolphinFileNameStorage();
    void testLastIdInitializationValue();
    void testLastIdGetterSetter();
    void testLastIdPersistenceOnLoad();
    void testFileInfoAfterCreation();

    // --- Auto-migration ---
    void testMigrationDisabledSkipsBackupAndResave();
    void testMigrationEnabledCreatesBackup();
    void testMigrationUpdatesFileVersion();
    void testMigrationCurrentVersionSkips();

    // save()/load()/autosave() version-warning, read-only, and dialog-driven paths
    // (Dialogs::provider()->choice() stands in for QMessageBox).
    //
    // Minimap geometry clamping lives in Minimap.qml's clampToParent() (QML/JS), which this
    // project has no QML-level harness to exercise directly; only the Settings round-trip half
    // is covered here, via TestQuickMinimap::testGeometryPersistsViaSettings.
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
    void testAutosaveFallsBackToAppDataForBundledExamplesDir();
    void testAutosaveRemovesPreviousFileWhenProjectDirChanges();
    void testAutosaveThrowsWhenFileCannotBeOpened();
    void testAutosaveThrowsWhenCommitFails();

private:
    QTemporaryDir m_tempDir;
};
