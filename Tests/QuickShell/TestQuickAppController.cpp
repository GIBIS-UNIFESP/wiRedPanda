// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickAppController.h"

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "App/Core/Enums.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Simulation/Simulation.h"

namespace {

/// Writes a minimal, valid, loadable circuit fixture (one switch, one LED) to \a path via
/// QuickWorkSpace -- not the Widgets-side WorkSpace, a real QWidget that aborts without a
/// QApplication (see Tests/QuickShell/TestQuickICHandlerSecurity.cpp's identical finding).
void writeFixture(const QString &path)
{
    QuickWorkSpace ws;
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(ws.save(path) == QuickWorkSpace::SaveOutcome::Saved, qPrintable(path));
}

} // namespace

// ===========================================================================
// Tab management
// ===========================================================================

void TestQuickAppController::testNewTabCreated()
{
    QuickAppController controller;
    controller.newTab();

    QCOMPARE(controller.tabCount(), 1);
    QVERIFY(controller.currentTab() != nullptr);
}

void TestQuickAppController::testCreateMultipleTabs()
{
    QuickAppController controller;
    controller.newTab();
    controller.newTab();
    controller.newTab();

    QCOMPARE(controller.tabCount(), 3);
}

void TestQuickAppController::testCloseTabReducesCount()
{
    QuickAppController controller;
    controller.newTab();
    controller.newTab();
    QCOMPARE(controller.tabCount(), 2);

    QVERIFY(controller.closeTab(0));
    QCOMPARE(controller.tabCount(), 1);
}

void TestQuickAppController::testCloseLastTab()
{
    QuickAppController controller;
    controller.newTab();

    QVERIFY(controller.closeTab(0));
    QCOMPARE(controller.tabCount(), 0);
    QVERIFY(controller.currentTab() == nullptr);
}

void TestQuickAppController::testSwitchTab()
{
    QuickAppController controller;
    controller.newTab();
    auto *tab0 = controller.currentTab();
    controller.newTab();
    auto *tab1 = controller.currentTab();
    QVERIFY(tab0 != tab1);

    controller.setCurrentIndex(0);
    QCOMPARE(controller.currentTab(), tab0);

    controller.setCurrentIndex(1);
    QCOMPARE(controller.currentTab(), tab1);
}

void TestQuickAppController::testCurrentTabReturnsActiveWorkspace()
{
    QuickAppController controller;
    controller.newTab();

    QVERIFY(controller.currentTab() != nullptr);
    QVERIFY(controller.currentTab()->canvas() != nullptr);
}

// ===========================================================================
// File operations
// ===========================================================================

void TestQuickAppController::testOpenFileCreatesNewTab()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");
    writeFixture(path);

    QuickAppController controller;
    controller.newTab();
    QCOMPARE(controller.tabCount(), 1);

    // openRecentFile() (QuickWorkspaceManager::loadPandaFile() underneath) always opens a
    // *new* tab, never replaces the current one.
    controller.openRecentFile(path);

    QCOMPARE(controller.tabCount(), 2);
    QCOMPARE(controller.currentTab()->canvas()->elements().size(), 2);
}

void TestQuickAppController::testSaveAndReload()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");

    QuickAppController controller;
    controller.newTab();
    controller.currentTab()->canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));

    controller.saveCurrentTabAs(path);
    QVERIFY(QFile::exists(path));

    // The tab's undo stack is clean (the element was added directly, bypassing commands), so
    // reloadFile()'s own closeTab() call won't hit the unsaved-changes confirmation dialog.
    controller.reloadFile();
    QCOMPARE(controller.currentTab()->canvas()->elements().size(), 1);
}

void TestQuickAppController::testReloadFile()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");
    writeFixture(path);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(path);
    QCOMPARE(controller.currentTab()->canvas()->elements().size(), 2);

    controller.reloadFile();
    QCOMPARE(controller.currentTab()->canvas()->elements().size(), 2);
}

void TestQuickAppController::testOpenRecentFileAddsToRecentFiles()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");
    writeFixture(path);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(path);

    QVERIFY(controller.recentFiles().contains(QFileInfo(path).absoluteFilePath()));
}

void TestQuickAppController::testOpenSameFileTwiceSwitchesTabInsteadOfDuplicating()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");
    writeFixture(path);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(path);
    QCOMPARE(controller.tabCount(), 2);
    auto *fileTab = controller.currentTab();

    controller.setCurrentIndex(0); // switch away
    QVERIFY(controller.currentTab() != fileTab);

    controller.openRecentFile(path); // reopening the same file switches back, doesn't duplicate

    QCOMPARE(controller.tabCount(), 2);
    QCOMPARE(controller.currentTab(), fileTab);
}

// ===========================================================================
// Simulation controls
// ===========================================================================

void TestQuickAppController::testPlayPauseToggle()
{
    QuickAppController controller;
    controller.newTab();

    QVERIFY(controller.isSimulationRunning()); // starts checked/running, mirrors actionPlay

    controller.setSimulationRunning(false);
    QVERIFY(!controller.isSimulationRunning());

    controller.setSimulationRunning(true);
    QVERIFY(controller.isSimulationRunning());
}

void TestQuickAppController::testRestartSimulation()
{
    QuickAppController controller;
    controller.newTab();

    controller.restartSimulation(); // must not crash on an empty canvas
    QVERIFY(controller.currentTab() != nullptr);
}

void TestQuickAppController::testBackgroundSimulation()
{
    QuickAppController controller;
    controller.newTab();
    QVERIFY(!controller.isBackgroundSimulationEnabled());

    controller.setBackgroundSimulationEnabled(true);
    QVERIFY(controller.isBackgroundSimulationEnabled());

    // With background simulation enabled, losing window focus does NOT pause the current
    // tab's simulation.
    controller.setSimulationRunning(true);
    controller.handleWindowActiveChanged(false); // lost focus
    QVERIFY(controller.currentTab()->canvas()->simulation()->isRunning());

    // With it disabled (the default), losing focus pauses it, and regaining focus resumes it.
    controller.setBackgroundSimulationEnabled(false);
    controller.handleWindowActiveChanged(false);
    QVERIFY(!controller.currentTab()->canvas()->simulation()->isRunning());

    controller.handleWindowActiveChanged(true);
    QVERIFY(controller.currentTab()->canvas()->simulation()->isRunning());
}

// ===========================================================================
// Edit/Transform delegation to the active tab
// ===========================================================================

void TestQuickAppController::testUndoRedoViaController()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas->receiveCommand(new CanvasAddItemsCommand({sw}, canvas));
    QCOMPARE(canvas->elements().size(), 1);

    controller.undo();
    QCOMPARE(canvas->elements().size(), 0);

    controller.redo();
    QCOMPARE(canvas->elements().size(), 1);
}

void TestQuickAppController::testDeleteSelectionViaController()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas->receiveCommand(new CanvasAddItemsCommand({sw}, canvas));
    sw->setSelected(true);

    controller.deleteSelection();
    QCOMPARE(canvas->elements().size(), 0);
}

void TestQuickAppController::testCopyPasteViaController()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas->receiveCommand(new CanvasAddItemsCommand({sw}, canvas));
    sw->setSelected(true);

    controller.copy();
    controller.paste();

    QCOMPARE(canvas->elements().size(), 2);
}

void TestQuickAppController::testSelectAllViaController()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();

    auto *sw0 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas->receiveCommand(new CanvasAddItemsCommand({sw0, sw1}, canvas));
    for (auto *elm : canvas->elements()) {
        elm->setSelected(false);
    }

    controller.selectAll();
    QCOMPARE(canvas->selectedElements().size(), 2);
}

void TestQuickAppController::testRotateFlipViaController()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas->receiveCommand(new CanvasAddItemsCommand({elm}, canvas));
    elm->setSelected(true);

    controller.rotateRight();
    QCOMPARE(elm->rotation(), 90.0);
    controller.rotateLeft();
    QCOMPARE(elm->rotation(), 0.0);

    QVERIFY(!elm->isFlippedX());
    controller.flipHorizontal();
    QVERIFY(elm->isFlippedX());

    QVERIFY(!elm->isFlippedY());
    controller.flipVertical();
    QVERIFY(elm->isFlippedY());
}

// ===========================================================================
// Mute
// ===========================================================================

void TestQuickAppController::testMuteTogglePersistsPerTab()
{
    QuickAppController controller;
    controller.newTab();
    QVERIFY(!controller.isMuted());

    controller.setMuted(true);
    QVERIFY(controller.isMuted());

    // Mute is genuinely per-tab state (Simulation::isUserMuted()) -- a fresh tab starts unmuted,
    // and switching back to the muted one resyncs the property to its own state.
    controller.newTab();
    QVERIFY(!controller.isMuted());

    controller.setCurrentIndex(0);
    QVERIFY(controller.isMuted());
}

// ===========================================================================
// Window title
// ===========================================================================

void TestQuickAppController::testWindowTitleFallsBackToAppNameWithNoTabs()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());

    const QString title = controller.windowTitle();
    QVERIFY(title.contains(QStringLiteral("wiRedPanda")));
    QVERIFY(!title.contains(QStringLiteral("New Project")));
}

void TestQuickAppController::testWindowTitleShowsNewProjectAndAppName()
{
    QuickAppController controller;
    controller.newTab();

    const QString title = controller.windowTitle();
    QVERIFY2(title.contains(QStringLiteral("New Project")), qPrintable(title));
    QVERIFY2(title.contains(QStringLiteral("wiRedPanda")), qPrintable(title));
    QVERIFY(!title.contains(QStringLiteral("*")));
}

void TestQuickAppController::testWindowTitleShowsAsteriskWhenModified()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, canvas));

    QVERIFY2(controller.windowTitle().contains(QStringLiteral("*")), qPrintable(controller.windowTitle()));
}

void TestQuickAppController::testWindowTitleReflectsOpenedFileName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.path() + "/my_circuit.panda";
    writeFixture(path);

    QuickAppController controller;
    controller.openRecentFile(path);

    QVERIFY2(controller.windowTitle().contains(QStringLiteral("my_circuit")), qPrintable(controller.windowTitle()));
}

// ===========================================================================
// Theme / language
// ===========================================================================

void TestQuickAppController::testThemeSwitchingUpdatesThemeManagerAndDarkThemeFlag()
{
    QuickAppController controller;
    const Theme original = ThemeManager::theme();

    controller.setThemeInt(static_cast<int>(Theme::Dark));
    QCOMPARE(ThemeManager::theme(), Theme::Dark);
    QCOMPARE(controller.themeInt(), static_cast<int>(Theme::Dark));

    controller.setThemeInt(static_cast<int>(Theme::Light));
    QCOMPARE(ThemeManager::theme(), Theme::Light);
    QVERIFY(!controller.isDarkTheme());

    controller.setThemeInt(static_cast<int>(Theme::System));
    QCOMPARE(ThemeManager::theme(), Theme::System);

    // ThemeManager::theme() is process-global state -- restore it so later tests in this same
    // binary run aren't affected by whatever this test last set it to.
    controller.setThemeInt(static_cast<int>(original));
}

void TestQuickAppController::testLanguageSwitchUpdatesCurrentLanguageAndRestores()
{
    QuickAppController controller;
    controller.switchLanguage("en");
    QCOMPARE(controller.currentLanguage(), QString("en"));

    controller.switchLanguage("pt_BR");
    QCOMPARE(controller.currentLanguage(), QString("pt_BR"));

    controller.switchLanguage("en");
    QCOMPARE(controller.currentLanguage(), QString("en"));
}

void TestQuickAppController::testLanguagesListIncludesEnglish()
{
    QuickAppController controller;
    const auto languages = controller.languages();
    QVERIFY(!languages.isEmpty());

    bool foundEnglish = false;
    for (const auto &entry : languages) {
        if (entry.code() == QStringLiteral("en")) {
            foundEnglish = true;
            QVERIFY(!entry.displayName().isEmpty());
        }
    }
    QVERIFY(foundEnglish);
}

// ===========================================================================
// Shortcuts help / Learn menu / Examples
// ===========================================================================

void TestQuickAppController::testShortcutsHelpHtmlCoversRealBindings()
{
    QuickAppController controller;
    const QString html = controller.shortcutsHelpHtml();

    QVERIFY(html.contains("Ctrl+X / C / V / D")); // Cut/Copy/Paste/Duplicate row
    QVERIFY(html.contains("Ctrl+Z"));
    QVERIFY(html.contains("Ctrl+A"));
    QVERIFY(html.contains("Ctrl+R"));
    QVERIFY(html.contains("Ctrl+H"));
    QVERIFY(html.contains("Ctrl+Shift+F"));
}

void TestQuickAppController::testExercisesListReturnsBundledContent()
{
    QuickAppController controller;
    const auto exercises = controller.exercisesList();
    QVERIFY2(!exercises.isEmpty(), "bundled Exercises content should always be discoverable");
    for (const auto &entry : exercises) {
        QVERIFY(!entry.title().isEmpty());
        QVERIFY(!entry.path().isEmpty());
    }
}

void TestQuickAppController::testToursListReturnsBundledContent()
{
    QuickAppController controller;
    const auto tours = controller.toursList();
    QVERIFY2(!tours.isEmpty(), "bundled Tours content should always be discoverable");
    for (const auto &entry : tours) {
        QVERIFY(!entry.title().isEmpty());
        QVERIFY(!entry.path().isEmpty());
    }
}

void TestQuickAppController::testExamplesListStructureAndOpenAddsTab()
{
    QuickAppController controller;
    const auto examples = controller.examplesList();
    if (examples.isEmpty()) {
        QSKIP("Examples/ directory not resolvable in this test environment");
    }

    const auto &first = examples.first();
    QVERIFY2(!first.title().contains(QStringLiteral(".panda")), qPrintable(first.title()));
    QVERIFY2(!first.title().contains(QLatin1Char('-')), qPrintable(first.title()));
    QVERIFY2(first.path().endsWith(QStringLiteral(".panda")), qPrintable(first.path()));

    controller.newTab();
    const int before = controller.tabCount();
    controller.openRecentFile(first.path());
    QCOMPARE(controller.tabCount(), before + 1);
}
