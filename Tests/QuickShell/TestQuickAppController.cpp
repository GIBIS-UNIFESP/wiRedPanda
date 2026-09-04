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
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
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

void TestQuickAppController::testMoveTabReordersList()
{
    QuickAppController controller;
    controller.newTab();
    auto *tab0 = controller.currentTab();
    controller.newTab();
    auto *tab1 = controller.currentTab();
    controller.newTab();
    auto *tab2 = controller.currentTab();

    // Drag tab 0 to land after tab 1: [tab0, tab1, tab2] -> [tab1, tab0, tab2].
    controller.moveTab(0, 1);

    QCOMPARE(controller.tabCount(), 3);
    QCOMPARE(controller.tabAt(0), tab1);
    QCOMPARE(controller.tabAt(1), tab0);
    QCOMPARE(controller.tabAt(2), tab2);
}

void TestQuickAppController::testMoveTabTracksCurrentTabAcrossReorder()
{
    QuickAppController controller;
    controller.newTab();
    auto *tab0 = controller.currentTab();
    controller.newTab();
    controller.newTab();

    // tab0 (still the active tab from before the other two newTab() calls moved it) sits at
    // index 0; moving it to the end must keep it the active tab, just at its new index.
    controller.setCurrentIndex(0);
    QCOMPARE(controller.currentTab(), tab0);

    controller.moveTab(0, 2);

    QCOMPARE(controller.currentTab(), tab0);
    QCOMPARE(controller.currentIndex(), 2);
    QCOMPARE(controller.tabAt(2), tab0);
}

void TestQuickAppController::testMoveTabOutOfRangeIsNoOp()
{
    QuickAppController controller;
    controller.newTab();
    auto *tab0 = controller.currentTab();
    controller.newTab();
    auto *tab1 = controller.currentTab();

    controller.moveTab(-1, 1);
    controller.moveTab(0, 5);
    controller.moveTab(0, 0);

    QCOMPARE(controller.tabCount(), 2);
    QCOMPARE(controller.tabAt(0), tab0);
    QCOMPARE(controller.tabAt(1), tab1);
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

void TestQuickAppController::testOpenDroppedPandaFileOpensLocalPandaFile()
{
    // Main.qml's DropArea has no dedicated drop-event type to validate a dropped URL against,
    // so the validation (local file, case-insensitive .panda suffix) lives in this
    // Q_INVOKABLE instead, matching this chrome's no-business-logic-in-QML convention.
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("dropped.panda");
    writeFixture(path);

    QuickAppController controller;
    controller.newTab();
    QCOMPARE(controller.tabCount(), 1);

    QVERIFY(controller.openDroppedPandaFile({QUrl::fromLocalFile(path)}));

    QCOMPARE(controller.tabCount(), 2);
    QCOMPARE(controller.currentTab()->canvas()->elements().size(), 2);
}

void TestQuickAppController::testOpenDroppedPandaFileSkipsNonLocalAndNonPandaUrls()
{
    QuickAppController controller;
    controller.newTab();
    QCOMPARE(controller.tabCount(), 1);

    QVERIFY(!controller.openDroppedPandaFile({}));
    QVERIFY(!controller.openDroppedPandaFile({QUrl("https://example.com/circuit.panda")}));

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString txtPath = tmpDir.filePath("notes.txt");
    QFile txtFile(txtPath);
    QVERIFY(txtFile.open(QIODevice::WriteOnly));
    txtFile.write("not a circuit");
    txtFile.close();
    QVERIFY(!controller.openDroppedPandaFile({QUrl::fromLocalFile(txtPath)}));

    QCOMPARE(controller.tabCount(), 1); // nothing opened, no new tab
}

void TestQuickAppController::testOpenDroppedPandaFileUsesFirstMatchingUrl()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString firstPath = tmpDir.filePath("first.panda");
    writeFixture(firstPath);
    const QString secondPath = tmpDir.filePath("second.panda");
    writeFixture(secondPath);

    QuickAppController controller;
    controller.newTab();

    // A non-local URL is skipped; of the two valid local .panda files, the first one in the
    // list wins -- mirrors droppedPandaFile()'s own "return on first match" loop.
    const QList<QUrl> urls{QUrl("ftp://example.com/unsupported.panda"),
                            QUrl::fromLocalFile(firstPath),
                            QUrl::fromLocalFile(secondPath)};
    QVERIFY(controller.openDroppedPandaFile(urls));
    QCOMPARE(controller.currentFile().absoluteFilePath(), firstPath);
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

void TestQuickAppController::testRecentFileBaseNameStripsDirectoryFromPath()
{
    // The Recent Files menu shows only this -- Main.qml's Repeater delegate -- so a full path
    // never leaks onto the label, matching MainWindow::updateRecentFileActions()'s own
    // QFileInfo(files.at(i)).fileName() exactly.
    QCOMPARE(QuickAppController::recentFileBaseName("/home/user/circuits/adder.panda"), QString("adder.panda"));
    QCOMPARE(QuickAppController::recentFileBaseName("adder.panda"), QString("adder.panda"));
}

void TestQuickAppController::testSaveFileShowsStatusMessage()
{
    // Mirrors WorkspaceManager's own m_host.showStatusMessage(tr("File saved successfully."),
    // 4000) call -- QuickWorkspaceManager::save() now threads this through its host too.
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");

    QuickAppController controller;
    controller.newTab();
    QVERIFY(controller.statusMessage().isEmpty());

    controller.saveCurrentTabAs(path);

    QCOMPARE(controller.statusMessage(), QString("File saved successfully."));
}

void TestQuickAppController::testOpenRecentFileShowsStatusMessage()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");
    writeFixture(path);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(path);

    QCOMPARE(controller.statusMessage(), QString("File loaded successfully."));
}

// ===========================================================================
// IC toolbar-button state (icButtonsVisible/icButtonsEnabled)
// ===========================================================================

void TestQuickAppController::testICButtonsVisibleForRootTab()
{
    QuickAppController controller;
    controller.newTab();

    QVERIFY(controller.icButtonsVisible());
}

void TestQuickAppController::testICButtonsHiddenForInlineICTab()
{
    // Mirrors MainWindow.cpp's setICButtonsVisible(!newTab->isInlineIC()) tab-switch call:
    // an inline IC tab has no project file/directory of its own for Add/Remove/
    // Make-Self-Contained to operate on.
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString fixturePath = tmpDir.filePath("ic_source.panda");
    writeFixture(fixturePath);
    QFile fixtureFile(fixturePath);
    QVERIFY(fixtureFile.open(QIODevice::ReadOnly));
    const QByteArray blob = fixtureFile.readAll();
    fixtureFile.close();

    QuickAppController controller;
    controller.newTab();
    QVERIFY(controller.icButtonsVisible());

    auto *canvas = controller.currentTab()->canvas();
    canvas->icRegistry()->setBlob("ic_test", blob);
    auto *ic = new IC();
    ic->setBlobName("ic_test");
    ic->loadFromBlob(blob, tmpDir.path());
    canvas->receiveCommand(new CanvasAddItemsCommand({ic}, canvas));

    controller.openICInTab("ic_test", ic->id(), blob);

    QVERIFY(controller.currentTab()->isInlineIC());
    QVERIFY(!controller.icButtonsVisible());

    controller.setCurrentIndex(0); // back to the root tab
    QVERIFY(controller.icButtonsVisible());
}

void TestQuickAppController::testICButtonsEnabledFollowsFileSavedState()
{
    // Mirrors MainWindow::refreshICButtonsEnabled(): importing a file-based IC needs a real,
    // readable project file to copy into -- a brand-new, never-saved tab has none.
    QuickAppController controller;
    controller.newTab();

    QVERIFY(!controller.icButtonsEnabled());

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    controller.saveCurrentTabAs(tmpDir.filePath("circuit.panda"));

    QVERIFY(controller.icButtonsEnabled());
}

void TestQuickAppController::testICButtonsRetainStateAfterClosingLastTab()
{
    // Mirrors MainWindow::onCurrentTabChanged()'s own no-tab branch: it returns *before*
    // touching setICButtonsVisible()/refreshICButtonsEnabled(), so closing the last tab leaves
    // the IC toolbar buttons in whatever state the last real tab set, rather than resetting them.
    //
    // This specifically needs the last tab to be a NON-inline tab (icButtonsVisible() == true
    // beforehand): a naive `setICButtonsVisible(tab && !tab->isInlineIC())` evaluates false
    // whenever tab is nullptr (which it is once currentTabChanged(nullptr) fires for the
    // last-closed tab), incorrectly forcing a true value back to false here -- a false-to-false
    // close (e.g. starting from an inline-IC tab) can't distinguish correct behavior from that.
    QuickAppController controller;
    controller.newTab();
    QVERIFY(controller.icButtonsVisible());

    QVERIFY(controller.closeTab(0));
    QCOMPARE(controller.tabCount(), 0);
    QVERIFY(controller.currentTab() == nullptr);

    QVERIFY2(controller.icButtonsVisible(), "closing the last (non-inline) tab must not force icButtonsVisible false");
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
// Tab tooltip file path (tabFilePath)
// ===========================================================================

void TestQuickAppController::testTabFilePathReturnsEmptyForNullTab()
{
    QuickAppController controller;
    QVERIFY(controller.tabFilePath(nullptr).isEmpty());
}

void TestQuickAppController::testTabFilePathReturnsEmptyForUnsavedTab()
{
    QuickAppController controller;
    controller.newTab();

    QVERIFY(controller.tabFilePath(controller.currentTab()).isEmpty());
}

void TestQuickAppController::testTabFilePathReturnsEmptyForInlineICTab()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString fixturePath = tmpDir.filePath("ic_source.panda");
    writeFixture(fixturePath);
    QFile fixtureFile(fixturePath);
    QVERIFY(fixtureFile.open(QIODevice::ReadOnly));
    const QByteArray blob = fixtureFile.readAll();
    fixtureFile.close();

    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    canvas->icRegistry()->setBlob("ic_test", blob);
    auto *ic = new IC();
    ic->setBlobName("ic_test");
    ic->loadFromBlob(blob, tmpDir.path());
    canvas->receiveCommand(new CanvasAddItemsCommand({ic}, canvas));

    controller.openICInTab("ic_test", ic->id(), blob);
    QVERIFY(controller.currentTab()->isInlineIC());

    QVERIFY(controller.tabFilePath(controller.currentTab()).isEmpty());
}

void TestQuickAppController::testTabFilePathReturnsAbsolutePathForSavedTab()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");

    QuickAppController controller;
    controller.newTab();
    controller.saveCurrentTabAs(path);

    QCOMPARE(controller.tabFilePath(controller.currentTab()), QFileInfo(path).absoluteFilePath());
}

void TestQuickAppController::testTabFilePathSurvivesFileDeletedFromDisk()
{
    // Mirrors WorkspaceManager::setCurrentFile()'s setTabToolTip(): the tooltip is set once and
    // never re-validated against the filesystem afterwards, so it keeps showing the last-known
    // path even if that file is later deleted -- proving tabFilePath() doesn't re-check
    // QFileInfo::exists() live on every call.
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.filePath("circuit.panda");

    QuickAppController controller;
    controller.newTab();
    controller.saveCurrentTabAs(path);
    const QString expectedPath = QFileInfo(path).absoluteFilePath();

    QVERIFY(QFile::remove(path));

    QCOMPARE(controller.tabFilePath(controller.currentTab()), expectedPath);
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
// Show Gates / Show Wires
// ===========================================================================

void TestQuickAppController::testGatesVisibleWiresVisibleTogglePersistPerTab()
{
    QuickAppController controller;
    controller.newTab();
    QVERIFY(controller.isGatesVisible());
    QVERIFY(controller.isWiresVisible());

    controller.setGatesVisible(false);
    controller.setWiresVisible(false);
    QVERIFY(!controller.isGatesVisible());
    QVERIFY(!controller.isWiresVisible());

    // Per-tab CanvasItem state, like mute -- a fresh tab starts fully visible, and switching
    // back to the first tab resyncs both properties to its own state.
    controller.newTab();
    QVERIFY(controller.isGatesVisible());
    QVERIFY(controller.isWiresVisible());

    controller.setCurrentIndex(0);
    QVERIFY(!controller.isGatesVisible());
    QVERIFY(!controller.isWiresVisible());
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
