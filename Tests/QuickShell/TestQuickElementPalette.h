// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Phase 7e (part 6) of the qtquick-rewrite plan: real coverage for QuickElementPalette
/// (App/QuickShell/Chrome/QuickElementPalette) and QuickAppController::addElementToCurrentTab(),
/// the actual, single shared entry point every palette-add gesture funnels through --
/// search-box-Enter (ElementPalette.qml's Keys.onReturnPressed), double-click
/// (PaletteItemDelegate.qml's onDoubleClicked), and real drag-and-drop (Main.qml's DropArea)
/// all call AppController.addElementToCurrentTab(type, icFileName, isEmbedded, x, y) with no
/// other logic in between -- confirmed by reading all three QML call sites, not assumed. This
/// closes TestMainWindowGui.cpp's testDragElementFromPalette()/testPaletteDoubleClickAddsElement()
/// intent without needing to drive real QML MouseArea drag gestures or double-click events,
/// which none of Phase 7's established techniques (direct API calls, QCoreApplication::sendEvent
/// synthesis) reach into QML-authored components for.
///
/// Neither QuickElementPalette nor addElementToCurrentTab() had any permanent test before this
/// sub-step, despite being fully ported since Phase 4 sub-step 4.
///
/// Phase 7f follow-up (Tests/Integration/TestICInline.cpp's "Batch 8 -- UI widgets" disposition,
/// ~19 tests): the 3 testSelectionCapabilities*/testContextMenuICActionConditions tests are
/// pure computeCapabilities() domain logic (KEEP, no Quick porting needed -- already covered by
/// the dedicated Tests/Unit/Ui/TestSelectionCapabilities.cpp regardless of UI framework). The 2
/// testElementLabelMime*/5 testSceneDrop*(MIME-specific)/3 testICDropZone*/1
/// testTrashButtonDragAcceptance tests all test Widgets' QMimeData-based drag-and-drop transport
/// (a custom "wpanda/x-dnditemdata"/"application/x-wiredpanda-dragdrop" binary MIME format) --
/// confirmed by reading Main.qml/PaletteItemDelegate.qml that Quick's own DnD never serializes
/// anything: DropArea.onDropped reads drop.source.modelData directly, an in-process live
/// reference. No MIME format exists to test on this side; ICDropZone/TrashButton's underlying
/// *actions* (embedICByFile()/extractICByBlobName()/removeICFile()) are already covered by
/// TestCanvasEmbeddedIC (Phase 7e-2). Two of the 5 testSceneDrop* tests' real *semantic* intent
/// (not their MIME mechanism) still applies to addElementFromPalette()'s own embedded-IC branch,
/// though, and are added below (missing-blob no-op, non-IC ignores the embedded flag) -- along
/// with 2 real testElementPaletteUpdateEmbeddedICList* scenarios (dedup-by-blob-name, a real
/// add/remove/refresh round trip) not covered by this file's original 7e-6 pass.
///
/// Phase 7h follow-up (Tests/Unit/Ui/TestElementPalette.cpp's own disposition, the last open
/// question from Phase 7f): testPaletteSearch() tests ElementFactory::nameMatchesSearch() --
/// KEEP as-is (QuickElementPalette::recomputeSearchResults() calls that exact static function
/// directly, no duplicate logic to test here; extracted from ElementPalette to ElementFactory
/// in Phase 8c so it survives ElementPalette's own eventual deletion). testPaletteRebuild()/
/// testAccessibleNameSet() are
/// Widgets-widget-construction/accessibility tests with the same disposition already established
/// elsewhere in 7f (DELETE-CANDIDATE / real-but-unported accessibility gap, respectively).
/// testElementLabelHasDescriptiveTooltip() is real, portable, and was a genuine small gap in this
/// file's own 7e-6 pass -- confirmed by reading ElementLabel.cpp that its tooltip is built from
/// the exact same ElementFactory::description() call ElementDescriptor::tooltip() uses, so
/// testCategoryEntriesHaveDescriptiveTooltips() below closes it directly.
class TestQuickElementPalette : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testCategoryListsPopulatedAtConstruction();
    void testCategoryEntriesHaveDescriptiveTooltips();

    void testSearchEmptyTextYieldsNoResults();
    void testSearchMatchesByNameCaseInsensitive();
    void testSearchNoMatchYieldsEmptyResults();

    void testFirstSearchResultReturnsEmptyMapWhenNoMatch();
    void testFirstSearchResultReturnsMatchingEntryFields();

    void testUpdateICListScansPandaFilesExcludingCurrentAndHidden();
    void testUpdateICListClearsWhenCurrentFileDoesNotExist();

    void testUpdateEmbeddedICListReflectsRegistryBlobs();
    void testUpdateEmbeddedICListClearsWhenRegistryNull();

    void testSearchIncludesDynamicallyLoadedICEntries();

    void testAddElementToCurrentTabAddsBuiltinElement();
    void testAddElementToCurrentTabAddsEmbeddedIC();
    void testAddElementToCurrentTabWithMissingEmbeddedBlobIsNoOp();
    void testAddElementToCurrentTabIgnoresEmbeddedFlagForNonICTypes();

    void testUpdateEmbeddedICListDedupesByBlobName();
    void testUpdateEmbeddedICListReflectsRemovalAfterRemoveEmbeddedIC();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
