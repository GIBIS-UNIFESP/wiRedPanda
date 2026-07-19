// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Phase 7e (part 3) of the qtquick-rewrite plan: real coverage for inline-IC-tab and
/// embedded-IC save/reload behavior, mirroring the corresponding slice of
/// Tests/Integration/TestMainWindowGui.cpp's "Embedded IC edge cases"/"Inline IC tab
/// operations" sections. Driven directly through QuickWorkspaceManager's real methods
/// (openICInTab(), saveFile()), not synthesized QKeyEvents.
///
/// A real gap found while scoping this sub-step, since fixed: CanvasICRegistry gained a
/// blobRenamed signal (mirroring ICRegistry::blobRenamed exactly -- the Widgets original is
/// already a QObject, so there was no real design fork here) and QuickWorkspaceManager now
/// connects to it per-tab in createNewTab(), retitling any open inline-IC tab tracking the
/// renamed blob. testInlineTabTitleUpdatesAfterBlobRename below exercises this for real.
///
/// Phase 7e-7 follow-up: testInlineICDropSaveCloseReopen, since landed. The Widgets original's
/// own doc comment describes the bug it guards: stale icOpenRequested connections accumulating
/// because MainWindow's disconnectTab() never ran on close. That specific mechanism can't
/// recur here -- QuickWorkspaceManager::removeTabAt() erases the closed tab's unique_ptr
/// outright (a real ~QObject(), not a detached-and-reused QTabWidget page), and Qt's own
/// QObject destructor automatically disconnects every signal/slot connection involving it. What
/// *is* still worth a real, permanent test: the full drop-a-file-IC / auto-embed-on-save / close
/// / reopen scenario end to end -- QuickWorkSpace::save()'s inline-tab auto-embed-file-backed-ICs
/// block (a faithful, already-landed port of WorkSpace::save()'s identical block) had zero
/// coverage before this test.
///
/// Still deferred: testEmbeddedICCopyPastePreservesState (duplicates
/// TestCanvasEmbeddedIC::testEmbeddedICCopyPaste's intent exactly, Phase 7e-2).
class TestCanvasInlineIC : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testInlineTabDeduplication();
    void testInlineTabTitleUpdatesAfterBlobRename();
    void testEmbeddedICSaveReloadRoundTrip();
    void testInlineICSaveNoFileDialog();
    void testInlineICSaveMarksRootDirty();
    void testInlineICDropSaveCloseReopen();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
