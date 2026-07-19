// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7e (part 1) of the qtquick-rewrite plan: real coverage for QuickAppController's tab/
/// file/simulation lifecycle and its Edit/Transform-menu delegation to the active tab's
/// CanvasItem, mirroring the corresponding slice of Tests/Integration/TestMainWindowGui.cpp's
/// intent (138 tests total there -- this is deliberately the first, most directly portable
/// slice, not the whole file; see the plan's Phase 7 section for the rest of the breakdown).
///
/// Edit/Transform tests here are NOT re-testing CanvasCommands.h's own logic (TestCanvasCommands,
/// Phase 7b, already covers that exhaustively) -- they confirm QuickAppController correctly
/// routes each menu action to whichever tab is currently active, a distinct concern multi-tab
/// apps can get wrong even when the underlying command logic is correct.
///
/// Out of scope for this pass, left for later 7e sub-steps: every keyboard/mouse-synthesized
/// interaction test (needs the same real-QKeyEvent/QMouseEvent-into-CanvasItem technique Phase 1
/// established, a substantial effort of its own), context menu operations, embedded-IC scenarios
/// (CanvasICRegistry has no permanent test yet either -- a real, separate gap), drag-from-palette,
/// and every dialog/window-chrome-specific test (About, fullscreen, theme switching, language
/// change, toolbar/status-bar widget presence, Learn menu structure).
///
/// Phase 7e-8 follow-up, added to this same class rather than a new file (same class under
/// test): the window-title/theme/language/shortcuts-help/Learn-menu slice of the "dialog/
/// window-chrome" deferral above -- the part reachable through real QuickAppController state/
/// methods with no QQuickWindow needed. Genuinely un-testable-without-a-window pieces (About/
/// AboutQt dialogs, fullscreen toggle, the Report-Translation-Error menu item's reachability)
/// stay deferred; see the plan's Phase 7e-8+ entry.
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

    // --- File operations ---
    void testOpenFileCreatesNewTab();
    void testSaveAndReload();
    void testReloadFile();
    void testOpenRecentFileAddsToRecentFiles();
    void testOpenSameFileTwiceSwitchesTabInsteadOfDuplicating();

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
