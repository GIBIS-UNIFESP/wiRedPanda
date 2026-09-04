// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Coverage for inline-IC-tab and embedded-IC save/reload behavior, driven directly through
/// QuickWorkspaceManager's real methods (openICInTab(), saveFile()), not synthesized QKeyEvents.
///
/// CanvasICRegistry emits blobRenamed (mirroring ICRegistry::blobRenamed), and
/// QuickWorkspaceManager connects to it per-tab in createNewTab(), retitling any open inline-IC
/// tab tracking the renamed blob. testInlineTabTitleUpdatesAfterBlobRename exercises this.
///
/// testInlineICDropSaveCloseReopen covers the full drop-a-file-IC / auto-embed-on-save / close /
/// reopen scenario end to end. QuickWorkspaceManager::removeTabAt() erases the closed tab's
/// unique_ptr outright, and Qt's QObject destructor disconnects every signal/slot connection
/// involving it, so stale icOpenRequested connections cannot accumulate the way they could when
/// MainWindow reused a detached QTabWidget page.
///
/// CanvasItem emits icOpenRequested(elementId, blobName, filePath) from the branch that handles
/// IC::handleDoubleClick()'s requestOpenSubCircuit(id(), blobName(), file()) signal.
/// QuickWorkspaceManager connects to it per-tab in createNewTab(), resolving it exactly the way
/// SceneUiBinder.cpp's Scene::icOpenRequested connection does: an embedded IC's blob opens
/// inline (openICInTab()); a file-backed IC's path opens as a new top-level tab (loadPandaFile()).
/// testDoubleClickEmbeddedICOpensInlineTab / testDoubleClickFileBackedICOpensNewTab drive the
/// real double-click gesture via QCoreApplication::sendEvent() through QuickWorkspaceManager end
/// to end.
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
    void testDoubleClickEmbeddedICOpensInlineTab();
    void testDoubleClickFileBackedICOpensNewTab();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
