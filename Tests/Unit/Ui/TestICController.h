// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class IC;
class WorkSpace;

class TestICController : public QObject
{
    Q_OBJECT

private:
    // Members (not free functions) so IC's "friend class TestICController;" grant covers
    // direct access to IC::m_file for building a file-backed IC test fixture.
    IC *selectBareIC(WorkSpace &tab);
    IC *selectFileBackedIC(WorkSpace &tab, const QString &fileName);

private slots:

    void testEmbedSelectedICNoOpWithNoCurrentTab();
    void testEmbedSelectedICNoOpWithEmptySelection();
    void testEmbedSelectedICNoOpWithNonICSelected();
    void testEmbedSelectedICNoOpWithEmptyFileName();
    void testEmbedSelectedICNoOpWhenProjectNotSavedAndDeclined();
    void testEmbedSelectedICWarnsWhenFileMissing();
    void testEmbedSelectedICBlobNameCollisionCancelled();
    void testEmbedSelectedICSucceeds();

    void testExtractSelectedICNoOpWhenNotEmbedded();
    void testExtractSelectedICNoOpWhenProjectNotSavedAndDeclined();
    void testExtractSelectedICNoOpWhenDialogCancelled();
    void testExtractSelectedICSucceeds();

    void testEmbedICByFileNoOpWithNoCurrentTab();
    void testEmbedICByFileNoOpWhenProjectNotSavedAndDeclined();
    void testEmbedICByFileWarnsWhenFileMissing();
    void testEmbedICByFileBlobNameCollisionCancelled();
    void testEmbedICByFileSucceeds();

    void testExtractICByBlobNameNoOpWhenBlobUnknown();
    void testExtractICByBlobNameNoOpWhenProjectNotSavedAndDeclined();
    void testExtractICByBlobNameNoOpWhenDialogCancelled();
    void testExtractICByBlobNameSucceeds();

    void testMakeSelfContainedNoOpWithNoFileBackedICs();
    void testMakeSelfContainedNoOpWhenProjectNotSavedAndDeclined();
    void testMakeSelfContainedSkipsNonFileBackedElements();
    void testMakeSelfContainedWarnsWhenFileMissing();
    void testMakeSelfContainedReportsPartialCompletionOnCancelledCollision();
    void testMakeSelfContainedSucceeds();

    void testAddICFromFileNoOpWhenSaveRequestedButStillUnsaved();

    void testAddEmbeddedICFromFileNoOpWithNoCurrentTab();
    void testAddEmbeddedICFromFileNoOpWhenDialogCancelled();
    void testAddEmbeddedICFromFileWarnsWhenFileUnreadable();
    void testAddEmbeddedICFromFileSucceeds();
    void testAddEmbeddedICFromFileBlobNameCollisionCancelled();

    void testRemoveEmbeddedICNoOpWithNoCurrentTab();
    void testRemoveEmbeddedICSucceeds();

    void testAddICFromFileNoOpWithNoCurrentTab();
    void testAddICFromFileNoOpWhenProjectNotSavedAndDeclined();
    void testAddICFromFileNoOpWhenDialogCancelled();
    void testAddICFromFileCopiesFile();
    void testAddICFromFileConflictKeepExisting();
    void testAddICFromFileConflictReplace();
    void testAddICFromFileConflictCancelled();

    void testShowRemoveICHintShowsInfo();

    void testRemoveICFileNoOpWithNoCurrentTab();
    void testRemoveICFileThrowsWhenTrashFails();
    void testRemoveICFileDeletesInstancesAndSaves();

    void testResolveUniqueBlobNameCollisionCancelled();
};
