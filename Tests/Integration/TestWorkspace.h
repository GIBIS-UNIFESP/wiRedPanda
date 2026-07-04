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
    // (testSaveAddsExtensionIfMissing / testSaveDoesNotDuplicateExtension moved to
    //  TestFileDialogProvider as testWorkspaceManagerSave{AddsExtensionIfMissing,
    //  DoesNotDuplicateExtension} -- the .panda-suffix logic moved to
    //  WorkspaceManager::promptSavePath() when WorkSpace::save() became pure)
    void testLoadNonExistentFileThrows();
    void testSaveToInvalidPathThrows();
    void testSaveEmptyCircuit();

    // Regression: B24 — save reprompts via Save-As on PermissionsError (covered by
    //   TestFileDialogProvider — see testWorkspaceSaveRepromptsOnPermissionsErrorB24
    //   there; the retry loop moved to WorkspaceManager::save() when WorkSpace::save()
    //   became pure)

    // Regression: B11 — loadPandaFile closes orphan tab on throw (covered by
    //   TestMainWindowGui — see testLoadPandaFileClosesOrphanedTabB11 there).

    // Regression: B3 — flushPendingAutosave runs the deferred autosave
    void testFlushPendingAutosaveRunsImmediatelyB3();

    // Regression: B2 — autosave truncates on shrink (no leftover-tail garbage)
    void testAutosaveTruncatesOnShrinkB2();

private:
    QTemporaryDir m_tempDir;
};
