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
/// A real, confirmed architectural gap found while scoping this sub-step, deliberately NOT
/// fixed here (out of scope for a test-authoring pass -- it needs a design decision, not a
/// bounded bug fix): Widgets' WorkspaceManager keeps an open inline-IC tab's displayed title
/// in sync with its blob's name by connecting to ICRegistry::blobRenamed, a real Qt signal
/// (App/UI/WorkspaceManager.cpp's onBlobRenamed()). CanvasICRegistry is a plain, non-QObject
/// class (deliberately, since Phase 3) -- it has no signal to connect to, so
/// QuickWorkspaceManager has no equivalent, and an inline tab's title goes stale after its
/// blob is renamed. testInlineTabTitleUpdatesAfterBlobRename is deferred for exactly this
/// reason, not silently dropped.
///
/// Also deferred: testInlineICDropSaveCloseReopen (heavy QTabWidget close/reopen-specific
/// mechanics needing real adaptation to QuickWorkspaceManager::closeTab()/openICInTab(), a
/// substantial effort of its own) and testEmbeddedICCopyPastePreservesState (duplicates
/// TestCanvasEmbeddedIC::testEmbeddedICCopyPaste's intent exactly, Phase 7e-2).
class TestCanvasInlineIC : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testInlineTabDeduplication();
    void testEmbeddedICSaveReloadRoundTrip();
    void testInlineICSaveNoFileDialog();
    void testInlineICSaveMarksRootDirty();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
