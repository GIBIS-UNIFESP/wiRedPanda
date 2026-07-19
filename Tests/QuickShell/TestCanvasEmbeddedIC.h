// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Phase 7e (part 2) of the qtquick-rewrite plan: real coverage for embedded-IC behavior --
/// CanvasICRegistry's blob storage plus CanvasItem's copy/cut/paste/delete/rotate/undo paths
/// operating on embedded IC elements, and QuickICController's file<->embedded conversion
/// commands. Mirrors the corresponding slice of Tests/Integration/TestMainWindowGui.cpp's
/// "Embedded IC operations via keyboard"/"Embedded IC edge cases" sections and its three
/// dedicated embedICByFile/extractICByBlobName/removeICFile tests -- driven directly through
/// CanvasItem's/QuickICController's real action methods instead of synthesized QKeyEvent/
/// drag-and-drop, the same "bypass the UI trigger, call the real underlying method" pattern
/// every other Phase 7 sub-step has used.
///
/// CanvasICRegistry had zero permanent test coverage anywhere before this -- a real, standalone
/// gap noted in 7e-1's own follow-up list.
///
/// Out of scope for this pass, not silently dropped: `makeSelfContained()` and its ~6 Widgets
/// tests -- QuickICController.h's own doc comment names this a real, deliberate deferral (no
/// toolbar-button UI trigger exists yet), so there is nothing on the Quick side to test.
/// Inline-IC-tab tests (opening a sub-circuit in its own tab for editing) are a separate,
/// real follow-up -- QuickWorkspaceManager::openICInTab() exists and is unexercised by any
/// permanent test, but scoping it in here would make this sub-step too large.
/// testAddICDisabledWhenUnsavedC9/testEmbedExtractViaContextMenuCallback are QAction-enabled-
/// state / context-menu-specific and stay grouped with the other deferred keyboard/mouse and
/// context-menu tests from 7e-1's own note.
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

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
