// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Tests QuickAppController's tab/file/simulation lifecycle and its Edit/Transform-menu
/// delegation to the active tab's CanvasItem.
///
/// Edit/Transform tests here are not re-testing CanvasCommands.h's own logic (TestCanvasCommands
/// already covers that exhaustively) -- they confirm QuickAppController correctly routes each
/// menu action to whichever tab is currently active, a distinct concern multi-tab apps can get
/// wrong even when the underlying command logic is correct.
///
/// The window-title/theme/language/shortcuts-help/Learn-menu slice covers only what's reachable
/// through real QuickAppController state/methods with no QQuickWindow needed; About/AboutQt
/// dialogs, the fullscreen toggle, and the Report-Translation-Error menu item's reachability
/// need a real window and stay untested here.
class TestQuickAppController : public QObject
{
    Q_OBJECT

private slots:
    // --- Tab management ---
    void testNewTabCreated();
    void testCreateMultipleTabs();
    void testCloseTabReducesCount();
    void testCloseLastTab();
    void testSwitchTab();
    void testCurrentTabReturnsActiveWorkspace();
    void testMoveTabReordersList();
    void testMoveTabTracksCurrentTabAcrossReorder();
    void testMoveTabOutOfRangeIsNoOp();

    // --- File operations ---
    void testOpenFileCreatesNewTab();
    void testOpenDroppedPandaFileOpensLocalPandaFile();
    void testOpenDroppedPandaFileSkipsNonLocalAndNonPandaUrls();
    void testOpenDroppedPandaFileUsesFirstMatchingUrl();
    void testSaveAndReload();
    void testReloadFile();
    void testOpenRecentFileAddsToRecentFiles();
    void testOpenSameFileTwiceSwitchesTabInsteadOfDuplicating();
    void testRecentFileBaseNameStripsDirectoryFromPath();
    void testSaveFileShowsStatusMessage();
    void testOpenRecentFileShowsStatusMessage();

    // --- IC toolbar-button state (icButtonsVisible/icButtonsEnabled) ---
    void testICButtonsVisibleForRootTab();
    void testICButtonsHiddenForInlineICTab();
    void testICButtonsEnabledFollowsFileSavedState();
    void testICButtonsRetainStateAfterClosingLastTab();

    // --- Tab tooltip file path (tabFilePath) ---
    void testTabFilePathReturnsEmptyForNullTab();
    void testTabFilePathReturnsEmptyForUnsavedTab();
    void testTabFilePathReturnsEmptyForInlineICTab();
    void testTabFilePathReturnsAbsolutePathForSavedTab();
    void testTabFilePathSurvivesFileDeletedFromDisk();

    // --- Simulation controls ---
    void testPlayPauseToggle();
    void testRestartSimulation();
    void testBackgroundSimulation();

    // --- Edit/Transform delegation to the active tab ---
    void testUndoRedoViaController();
    void testDeleteSelectionViaController();
    void testCopyPasteViaController();
    void testSelectAllViaController();
    void testRotateFlipViaController();

    // --- Mute (genuinely per-tab state) ---
    void testMuteTogglePersistsPerTab();
    void testGatesVisibleWiresVisibleTogglePersistPerTab();

    // --- Window title ---
    void testWindowTitleFallsBackToAppNameWithNoTabs();
    void testWindowTitleShowsNewProjectAndAppName();
    void testWindowTitleShowsAsteriskWhenModified();
    void testWindowTitleReflectsOpenedFileName();

    // --- Theme / language ---
    void testThemeSwitchingUpdatesThemeManagerAndDarkThemeFlag();
    void testLanguageSwitchUpdatesCurrentLanguageAndRestores();
    void testLanguagesListIncludesEnglish();

    // --- Shortcuts help / Learn menu / Examples ---
    void testShortcutsHelpHtmlCoversRealBindings();
    void testExercisesListReturnsBundledContent();
    void testToursListReturnsBundledContent();
    void testExamplesListStructureAndOpenAddsTab();
};
