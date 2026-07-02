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
    void testNewTabsAreNumbered();
    void testTabsMovableAndMiddleClickCloses();
    void testRecoveredTabIsMarked();
    void testCloseTabReducesCount();
    void testSwitchTab();
    void testCurrentTabReturnsActiveWorkspace();

    // --- File operations ---

    void testOpenFile();
    void testOpenCreatesNewTab();
    void testSaveAndReload();
    void testSaveAsConflictBlocksSave();
    void testSaveAsConflictSwitchToTab();
    void testReloadFile();

    // --- Simulation controls ---

    void testPlayPauseToggle();
    void testRestartSimulation();
    void testTemporalModeSelector();
    void testTemporalModeReinitializesFeedback();
    void testWaveformDockOpensAndClosesCleanly();
    void testWaveformToolDiesWithItsTab();
    void testWaveformDockSwapsPerTabInstances();
    void testWatchAllSignalsRecordsOutputs();
    void testWatchICInternalsRecordsSignals();
    /// Regression: the viewer's auto-follow used a fixed-threshold check on the refresh
    /// timer, racing the paint that grows the canvas — any growth beyond the 4 px slack per
    /// tick disengaged it permanently. The sticky-tail follow (scroll bar rangeChanged /
    /// valueChanged) must pin the view to new data, release when the user scrolls back, and
    /// re-engage when scrolled to the right edge.
    void testWaveformViewerFollowsNewestData();

    // --- View toggles ---

    void testToggleWiresVisibility();
    void testToggleGatesVisibility();
    void testZoomInOutReset();
    void testZoomInAcceptsCtrlPlus();
    void testFastModeToggle();
    void testFullscreenToggle();
    void testLabelsUnderIconsToggle();
    void testUpdateChecksToggle();

    // --- Menu structure (#8: Exercises+Tours -> Learn; Report-Translation -> Help) ---

    void testLearnMenuHostsExercisesAndToursAsSubmenus();
    void testReportTranslationErrorReachableViaHelpMenu();

    // --- Element manipulation via keyboard ---

    void testDeleteViaKeyboard();
    void testDeleteViaKeyboardSurvivesMistranslatedShortcut();
    void testCopyPasteViaKeyboard();
    void testDuplicateViaKeyboard();
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
    void testSetupLanguageAutoDetectExactMatch();
    void testSetupLanguageAutoDetectBaseLanguageMatch();

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
    void testMakeSelfContainedPartialDoesNotClaimSuccess();
    void testMakeSelfContainedPromptsToSaveWhenUnsaved();
    void testEmbedExtractViaContextMenuCallback();
    void testDeleteLastEmbeddedInstanceUndo();
    void testReEmbedAfterExtract();
    void testInlineTabDeduplication();
    // Regression: renaming an IC's blob while its sub-circuit tab is open used to leave the
    // tab title showing the pre-rename name indefinitely (it was only ever set once, at
    // openICInTab() time).
    void testInlineTabTitleUpdatesAfterBlobRename();
    void testEmbeddedICSaveReloadRoundTrip();

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
    void testToolbarHasUndoRedoActions();
    void testStatusBarShowsZoomAndSelection();
    void testRedoAfterNewAction();
    void testWireCreationViaMouse();
    void testRubberBandSelection();
    void testDeleteConnection();
    void testCopyPasteConnectedCircuit();
    void testPropertyCycleMain();
    void testToggleInputDuringSimulation();
    void testInputButtonMomentary();
    void testInputButtonMousePressReleaseIsMomentary();
    void testRapidSimulationToggle();
    void testOpenSameFileTwice();
    void testOpenSameFileFromDifferentDirs();

    // --- Context menu operations ---

    void testContextMenuRealRightClickRotatesElement();
    void testContextMenuPasteOnEmpty();
    void testContextMenuRename();
    void testContextMenuChangeTrigger();
    void testContextMenuChangeColor();
    void testContextMenuChangeFrequency();

    // --- Keyboard shortcuts (additional) ---

    void testSearchFocusViaCtrlF();
    void testMorphPrevNextViaKeyboard();

    // --- Mouse interactions (additional) ---

    void testDoubleClickWireSplit();
    void testDragElementFromPalette();
    void testPaletteDoubleClickAddsElement();

    // --- View/Help operations ---

    void testThemeSwitching();
    void testAboutThisVersionDialog();
    void testOpenExample();
    void testWindowTitleShowsFile();
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
    void testWaveformOnEmptySceneShowsWarningHQ();

    // --- Deep-review regressions ---

    void testRecoveredAutosaveCountsAsModifiedF6();

    // --- MainWindow.cpp coverage completion ---

    void testConstructorWithFileNameArgumentLoadsFile();
    void testExamplesMenuHiddenWhenNoExamplesDirectoryFound();
    void testPopulateContentMenuOpenFolderAction();
    void testPopulateContentMenuOpenFolderActionShowsWarningWhenUnwritable();
    void testEditSubcircuitRequestedOpensICTab();
    void testReportTranslationErrorOpensWeblate();
    void testCloseEventClosesAfterConfirmedSave();
    void testCloseEventIgnoredWhenCancelled();
    void testWidgetAndDolphinHostAccessors();
    void testActionsAreNoOpsWithNoCurrentTab();
    void testAlignDistributeFlipZoomToFitActionsWithTab();
    void testExerciseOverlayDetachesAndReattachesAcrossTabSwitch();
    void testExportWrapperMethodsDelegateToController();
    void testRetranslateUiHandlesInlineIcAndElementsAndEngines();
    void testPopulateLanguageMenuSwitchesLanguage();
    void testWaveformActionRaisesExistingWindow();
    void testWindowActivateDeactivateTogglesSimulation();
    void testDolphinFileNameAccessors();
    void testActionICPreviewTogglesSetting();
    void testClickTargetDrivesEachKnownId();
    void testResolveTourTargetForEachKnownId();
    void testStartExerciseDrivesClickTargetsAndOverlayParenting();
    void testExerciseCloseRequestedDetachesOverlayFromOpenBewavedDolphin();
    void testStartTourDrivesClickTargetsAndOverlayParenting();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
