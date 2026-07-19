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
class TestQuickElementPalette : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testCategoryListsPopulatedAtConstruction();

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

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
