// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestQuickWorkSpace : public QObject
{
    Q_OBJECT

private slots:
    // Autosave triggering / naming / cleanup
    void testAutosaveTriggersOnCircuitChange();
    void testAutosaveSkippedWhenClean();
    void testAutosaveFileNamingConvention();
    void testAutosaveFilesUniqueAcrossWorkspaces();
    void testAutosaveDeletedOnExplicitSave();
    void testAutosaveRemovedOnCleanDestruction();
    void testFlushPendingAutosaveRunsImmediately();
    void testAutosaveTruncatesOnShrink();
    void testFirstSaveCopiesExternalAppearanceFile();
    void testResaveCopiesNewlyAddedDependency();

    // Save/load
    void testSaveToNewFile();
    void testSaveToExistingFileOverwrite();
    void testSavePreservesElementData();
    void testSaveEmptyCircuit();
    void testLoadFromValidFile();
    void testLoadNonExistentFileThrows();
    void testLoadCorruptedFileHandledGracefully();
    void testSaveToInvalidPathFails();

    // File/modified state tracking
    void testModifiedFlagAfterAddElement();
    void testModifiedFlagClearedAfterSave();
    void testFilePathUpdatedAfterSave();
    void testFilePathPreservedAfterLoad();
    void testDolphinFileNameStorage();
    void testLastIdGetterSetter();
    void testLastIdPersistenceOnLoad();
    void testFileInfoEmptyBeforeAnySaveOrLoad();

    // Version migration on load
    void testMigrationDisabledSkipsBackupAndResave();
    void testMigrationEnabledCreatesBackup();
    void testMigrationUpdatesFileVersion();
    void testMigrationCurrentVersionSkipsBackup();
};
