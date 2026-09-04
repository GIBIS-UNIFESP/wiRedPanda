// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Covers QuickElementPalette (App/QuickShell/Chrome/QuickElementPalette) and
/// QuickAppController::addElementToCurrentTab(), the single shared entry point every
/// palette-add gesture funnels through: search-box-Enter (ElementPalette.qml's
/// Keys.onReturnPressed), double-click (PaletteItemDelegate.qml's onDoubleClicked), and
/// drag-and-drop (Main.qml's DropArea) all call
/// AppController.addElementToCurrentTab(type, icFileName, isEmbedded, x, y) with no other logic
/// in between, so exercising that one function covers all three gestures without driving real
/// QML MouseArea drag or double-click events.
///
/// Quick's drag-and-drop never serializes anything -- DropArea.onDropped reads
/// drop.source.modelData directly, an in-process live reference -- so there is no MIME format to
/// test on this side of the port.
///
/// testElementLabelHasDescriptiveTooltip() relies on ElementLabel's tooltip being built from the
/// same ElementFactory::description() call each descriptor's own "tooltip" field uses, so
/// testCategoryEntriesHaveDescriptiveTooltips() below covers both.
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
    void testSearchMatchesInternalTypeKeywordEvenWhenDisplayNameDiffers();
    void testSearchMatchesFileBasedICFullFileNameIncludingExtension();
    void testSearchMatchesGenericIcKeywordRegardlessOfFileName();
    void testSearchByExtensionDoesNotMatchEmbeddedICEntries();

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
