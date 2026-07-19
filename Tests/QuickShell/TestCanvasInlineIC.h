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
/// Still deferred: testInlineICDropSaveCloseReopen (heavy QTabWidget close/reopen-specific
/// mechanics needing real adaptation to QuickWorkspaceManager::closeTab()/openICInTab(), a
/// substantial effort of its own) and testEmbeddedICCopyPastePreservesState (duplicates
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

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
