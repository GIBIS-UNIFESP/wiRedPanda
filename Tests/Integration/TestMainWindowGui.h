// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

/// GUI-level tests for MainWindow operations using simulated keyboard/mouse input.
class TestMainWindowGui : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // --- Tab management ---

    void testNewTabCreated();
    void testCreateMultipleTabs();
    void testCloseTabReducesCount();
    void testSwitchTab();
    void testCurrentTabReturnsActiveWorkspace();

    // --- File operations ---

    void testOpenFile();
    void testOpenCreatesNewTab();
    void testSaveAndReload();
    void testReloadFile();

    // --- Simulation controls ---

    void testPlayPauseToggle();
    void testRestartSimulation();

    // --- View toggles ---

    void testToggleWiresVisibility();
    void testToggleGatesVisibility();
    void testZoomInOutReset();
    void testFastModeToggle();
    void testFullscreenToggle();
    void testLabelsUnderIconsToggle();

    // --- Element manipulation via keyboard ---

    void testDeleteViaKeyboard();
    void testCopyPasteViaKeyboard();
    void testCutPasteViaKeyboard();
    void testUndoRedoViaKeyboard();
    void testSelectAllViaKeyboard();
    void testRotateRightViaKeyboard();
    void testRotateLeftViaKeyboard();
    void testFlipHorizontallyViaKeyboard();
    void testMuteViaKeyboard();
    void testPropertyCycleSecondary();

    // --- Element manipulation via mouse ---

    void testMouseClickSelectsElement();
    void testMouseCtrlClickMultiSelect();
    void testMouseDragMovesElement();
    void testMouseWheelZoom();
    void testMouseClickEmptyDeselects();
    void testMousePanWithSpace();

    // --- Export operations ---

    void testExportArduino();
    void testExportSystemVerilog();

    // --- Remaining MainWindow operations ---

    void testExitViaKeyboard();
    void testOpenRecentFile();
    void testBackgroundSimulation();
    void testOpenWaveformEditor();
    void testLanguageChange();

    // --- Help dialogs ---

    void testAboutDialog();
    void testAboutQtDialog();
    void testShortcutsDialog();

    // --- ElementEditor operations ---

    void testChangeTriggerKey();
    void testChangeSkin();
    void testDefaultSkin();
    void testChangeDelay();

    // --- Embedded IC operations via keyboard ---

    void testEmbeddedICCopyPaste();
    void testEmbeddedICCutPaste();
    void testEmbeddedICDeleteUndo();
    void testEmbeddedICSelectAllDeleteUndo();
    void testEmbeddedICRotatePreservesState();
    void testEmbeddedICSimulationAfterDeleteUndo();
    void testEmbeddedICMultipleTypesDeleteOne();

    // --- Embedded IC edge cases ---

    void testEmbeddedICCrossTabCopyPaste();
    void testMakeSelfContainedWithFileICs();
    void testMakeSelfContainedMixedScene();
    void testMakeSelfContainedLabelsPreserved();
    void testEmbedExtractViaContextMenuCallback();
    void testDeleteLastEmbeddedInstanceUndo();
    void testReEmbedAfterExtract();
    void testInlineTabDeduplication();
    void testEmbeddedICSaveReloadRoundTrip();
    void testEmbeddedICCopyPastePreservesState();

    // --- Inline IC tab operations ---

    void testInlineICDropSaveCloseReopen();
    void testInlineICSaveNoFileDialog();
    void testInlineICSaveMarksRootDirty();

    // --- Edge cases & missing coverage ---

    void testClearSelectionViaEscape();
    void testFlipVertically();
    void testCloseLastTab();
    void testEmptySceneDelete();
    void testEmptySceneCopyPaste();
    void testMaxZoom();
    void testMinZoom();
    void testUndoPastStack();
    void testRedoAfterNewAction();
    void testWireCreationViaMouse();
    void testRubberBandSelection();
    void testDeleteConnection();
    void testCopyPasteConnectedCircuit();
    void testPropertyCycleMain();
    void testToggleInputDuringSimulation();
    void testInputButtonMomentary();
    void testRapidSimulationToggle();
    void testOpenSameFileTwice();

    // --- Context menu operations ---

    void testContextMenuRotateLeft();
    void testContextMenuRotateRight();
    void testContextMenuCopy();
    void testContextMenuCut();
    void testContextMenuDelete();
    void testContextMenuPasteOnEmpty();
    void testContextMenuRename();
    void testContextMenuChangeTrigger();
    void testContextMenuChangeColor();
    void testContextMenuChangeFrequency();
    void testContextMenuMorphTo();

    // --- Keyboard shortcuts (additional) ---

    void testSearchFocusViaCtrlF();
    void testMorphPrevNextViaKeyboard();

    // --- Mouse interactions (additional) ---

    void testDoubleClickWireSplit();
    void testDragElementFromPalette();

    // --- View/Help operations ---

    void testThemeSwitching();
    void testAboutThisVersionDialog();
    void testOpenExample();
    void testMakeSelfContained();

    // --- ElementEditor operations ---

    void testElementLabelRename();
    void testElementColorChange();
    void testElementInputCountChange();

    // --- Embedded IC: embedICByFile / extractICByBlobName ---

    void testEmbedICByFileNoInstances();
    void testEmbedICByFileWithInstances();
    void testExtractICByBlobNameEndToEnd();

    // --- Sentry triage regressions ---

    void testSetupShortcutsRegistersOnceD18();
    void testAddICDisabledWhenUnsavedC9();
    void testRotateFlipZoomBreadcrumbsB23();
    void testLoadPandaFileClosesOrphanedTabB11();
    void testRemoveICFileIsUndoableA14();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};

