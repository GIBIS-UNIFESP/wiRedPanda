// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

class TestWorkspace : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    // Autosave Triggering Tests
    void testAutosaveTriggersOnCircuitChange();
    void testAutosaveSkippedWhenClean();
    void testAutosaveUpdatesSettings();
    void testAutosaveAfterElementAdd();
    void testAutosaveAfterElementModify();
    void testAutosaveAfterElementDelete();
    void testAutosaveSignalEmitted();
    void testMultipleAutosavesUpdateSettings();

    // Autosave File Naming Tests
    void testAutosaveFileCreatedInAppData();
    void testAutosaveFileNameFormatNewProject();
    void testAutosaveFileNameFormatSavedProject();
    void testAutosaveFileRandomSuffixGeneration();
    void testAutosavePathCreatedIfNotExists();
    void testAutosaveFileExtensionCorrect();
    void testAutosaveInCurrentDirForSavedProject();
    void testAutosaveInAppDataForNewProject();
    void testAutosaveFileTemplatePattern();
    void testMultipleAutosaveFilesNonConflicting();
    void testAutosaveFilePermissions();

    // Autosave Cleanup Tests
    void testAutosaveDeletedOnExplicitSave();
    void testAutosaveRemovedFromSettingsOnSave();
    void testRecoveredFileRemovedFromSettings();
    void testAutosaveCleanupAfterSaveAs();
    void testAutosaveListCorrectAfterCleanup();
    void testMultipleAutosavesCleanedUp();
    void testAutosaveCleanupWithEmptySettings();

    // File Operation Edge Cases
    void testSaveAddsExtensionIfMissing();
    void testSaveDoesNotDuplicateExtension();
    void testLoadNonExistentFileThrows();
    void testSaveToInvalidPathThrows();
    void testSaveEmptyCircuit();

    // Regression: B3 — flushPendingAutosave runs the deferred autosave
    void testFlushPendingAutosaveRunsImmediatelyB3();

    // Regression: B2 — autosave truncates on shrink (no leftover-tail garbage)
    void testAutosaveTruncatesOnShrinkB2();

private:
    QTemporaryDir m_tempDir;
};

