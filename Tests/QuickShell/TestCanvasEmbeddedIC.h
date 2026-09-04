// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Tests embedded-IC behavior -- CanvasICRegistry's blob storage plus CanvasItem's copy/cut/
/// paste/delete/rotate/undo paths operating on embedded IC elements, and QuickICController's
/// file<->embedded conversion commands. Mirrors the corresponding slice of
/// Tests/Integration/TestMainWindowGui.cpp's "Embedded IC operations via keyboard"/"Embedded IC
/// edge cases" sections and its embedICByFile/extractICByBlobName/removeICFile tests, driven
/// directly through CanvasItem's/QuickICController's real action methods instead of synthesized
/// QKeyEvent/drag-and-drop.
///
/// `addICFromFile()`/`embedSelectedIC()`/`extractSelectedIC()`/`makeSelfContained()`/
/// `addEmbeddedICFromFile()` are covered below by their core success/guard paths, not every
/// corresponding Widgets-side TestICController.cpp test, since most of those exercise
/// QMessageBox/QInputDialog-specific wording this port's DialogProvider abstraction already
/// covers once, generically, elsewhere.
class TestCanvasEmbeddedIC : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testEmbeddedICCopyPaste();
    void testEmbeddedICCutPaste();
    void testEmbeddedICDeleteUndo();
    void testEmbeddedICSelectAllDeleteUndo();
    void testEmbeddedICRotatePreservesState();
    void testEmbeddedICSimulationAfterDeleteUndo();
    void testEmbeddedICMultipleTypesDeleteOne();
    void testEmbeddedICCrossTabCopyPaste();

    void testEmbedICByFileNoInstances();
    void testEmbedICByFileWithInstances();
    void testExtractICByBlobNameEndToEnd();
    void testRemoveICFileIsUndoableA14();

    void testEmbedSelectedICNoOpWithNoSelection();
    void testEmbedSelectedICNoOpWhenNotFileBacked();
    void testEmbedSelectedICSucceeds();

    void testExtractSelectedICNoOpWhenNotEmbedded();
    void testExtractSelectedICSucceeds();

    void testMakeSelfContainedNoOpWithNoFileBasedICs();
    void testMakeSelfContainedEmbedsAllFileBasedICs();

    void testAddICFromFileCopiesFile();
    void testAddICFromFileNoOpWhenDialogCancelled();

    void testAddEmbeddedICFromFileSucceeds();
    void testAddEmbeddedICFromFileNoOpWhenDialogCancelled();

    void testRenameBlobUpdatesNestedMetadataReference();
    void testRegisterBlobInlinesFileBackedDependency();

    void testFileWatcherEmitsDefinitionChangedWithNoTargets();
    void testFileWatcherReloadsFileBackedICAndPushesUndoCommand();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
