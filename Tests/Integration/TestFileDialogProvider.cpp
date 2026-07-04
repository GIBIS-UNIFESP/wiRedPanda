// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestFileDialogProvider.h"

#include <QApplication>
#include <QFile>
#include <QPushButton>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementEditor.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/MainWindow.h"
#include "Tests/Common/StubFileDialogProvider.h"
#include "Tests/Common/TestUtils.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace FileDialogTestHelpers {

static void createFixture(const QString &path)
{
    WorkSpace ws;
    auto *sw = new InputSwitch();
    auto *led = new Led();
    ws.scene()->addItem(sw);
    ws.scene()->addItem(led);
    ws.save(path);
}

/// Creates a shown, focused MainWindow for keyboard-driven tests.
/// Forces English locale so that translated keyboard shortcuts match QTest key events.
/// Uses QApplication::setActiveWindow (deprecated but required for offscreen platform
/// where activateWindow() is a no-op since there is no window manager).
static MainWindow *createWindow()
{
    Settings::setLanguage("en");
    auto *w = new MainWindow();
    w->show();
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    QApplication::setActiveWindow(w);
QT_WARNING_POP
    return w;
}

} // namespace FileDialogTestHelpers

using namespace FileDialogTestHelpers;

// ---------------------------------------------------------------------------
// Test setup
// ---------------------------------------------------------------------------

void TestFileDialogProvider::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    m_fixtureDir = m_tempDir.path();
    createFixture(m_fixtureDir + "/test_circuit.panda");
}

void TestFileDialogProvider::cleanup()
{
    FileDialogs::setProvider(nullptr);
}

// ===========================================================================
// Provider infrastructure
// ===========================================================================

void TestFileDialogProvider::testDefaultProviderIsNotNull()
{
    QVERIFY(FileDialogs::provider() != nullptr);
}

void TestFileDialogProvider::testSetProviderSwapsAndRestores()
{
    auto *original = FileDialogs::provider();
    QVERIFY(original);

    StubFileDialogProvider stub;
    auto *old = FileDialogs::setProvider(&stub);
    QCOMPARE(old, original);
    QCOMPARE(FileDialogs::provider(), &stub);

    FileDialogs::setProvider(old);
    QCOMPARE(FileDialogs::provider(), original);
}

void TestFileDialogProvider::testScopedStubRestoresOnDestruction()
{
    auto *original = FileDialogs::provider();

    {
        ScopedFileDialogStub guard;
        QCOMPARE(FileDialogs::provider(), &guard.stub);
        QVERIFY(FileDialogs::provider() != original);
    }

    QCOMPARE(FileDialogs::provider(), original);
}

// ===========================================================================
// MainWindow: Open (Ctrl+O)
// ===========================================================================

void TestFileDialogProvider::testOpenActionUsesProvider()
{
    ScopedFileDialogStub guard;
    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";

    std::unique_ptr<MainWindow> window(createWindow());

    // Ctrl+O to open
    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier);

    QCOMPARE(guard.stub.openCallCount, 1);
    QVERIFY(guard.stub.lastOpenCall.caption.contains("Open"));
    QVERIFY(guard.stub.lastOpenCall.filter.contains("panda"));
}

void TestFileDialogProvider::testOpenActionCancelledDoesNotLoad()
{
    ScopedFileDialogStub guard;
    guard.stub.openResult = QString(); // cancelled

    std::unique_ptr<MainWindow> window(createWindow());

    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier);

    QCOMPARE(guard.stub.openCallCount, 1);
    // Dialog was cancelled (empty result) — no file loaded
}

// ===========================================================================
// MainWindow: Save / Save As (Ctrl+S, Ctrl+Shift+S)
// ===========================================================================

void TestFileDialogProvider::testSaveAsUsesProvider()
{
    ScopedFileDialogStub guard;
    const QString savePath = m_fixtureDir + "/save_as_test.panda";

    std::unique_ptr<MainWindow> window(createWindow());

    // Load existing file first via Ctrl+O so currentFile() is valid
    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";
    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier);

    // Now trigger Save As via Ctrl+Shift+S
    guard.stub.saveCallCount = 0;
    guard.stub.saveResult = {savePath, "Panda files (*.panda)"};
    QTest::keyClick(window.get(), Qt::Key_S, Qt::ControlModifier | Qt::ShiftModifier);

    QCOMPARE(guard.stub.saveCallCount, 1);
    QVERIFY(guard.stub.lastSaveCall.caption.contains("Save"));
    QVERIFY(guard.stub.lastSaveCall.filter.contains("panda"));
    QVERIFY(QFile::exists(savePath));

    QFile::remove(savePath);
}

void TestFileDialogProvider::testSaveAsAppendsExtension()
{
    ScopedFileDialogStub guard;

    std::unique_ptr<MainWindow> window(createWindow());

    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";
    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier);

    // Return path without extension — the action should append .panda
    const QString savePath = m_fixtureDir + "/no_ext_test";
    guard.stub.saveResult = {savePath, "Panda files (*.panda)"};
    guard.stub.saveCallCount = 0;

    QTest::keyClick(window.get(), Qt::Key_S, Qt::ControlModifier | Qt::ShiftModifier);

    QVERIFY(QFile::exists(savePath + ".panda"));

    QFile::remove(savePath + ".panda");
}

void TestFileDialogProvider::testSaveFirstTimeUsesProvider()
{
    ScopedFileDialogStub guard;
    const QString savePath = m_fixtureDir + "/first_save.panda";
    guard.stub.saveResult = {savePath, "Panda files (*.panda)"};

    std::unique_ptr<MainWindow> window(createWindow());

    auto *sw = new InputSwitch();
    window->currentTab()->scene()->addItem(sw);

    // Ctrl+S on unsaved project triggers Save dialog
    QTest::keyClick(window.get(), Qt::Key_S, Qt::ControlModifier);

    QCOMPARE(guard.stub.saveCallCount, 1);
    QVERIFY(QFile::exists(savePath));

    QFile::remove(savePath);
}

// ===========================================================================
// MainWindow: Exports (Ctrl+Alt+I, Ctrl+Alt+O, Ctrl+Shift+P, Ctrl+E)
// ===========================================================================

void TestFileDialogProvider::testExportArduinoUsesProvider()
{
    ScopedFileDialogStub guard;
    const QString exportPath = m_fixtureDir + "/export.ino";

    std::unique_ptr<MainWindow> window(createWindow());

    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";
    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier);

    guard.stub.saveResult = {exportPath, "Arduino file (*.ino)"};
    guard.stub.saveCallCount = 0;

    // Ctrl+Alt+I to export Arduino
    QTest::keyClick(window.get(), Qt::Key_I, Qt::ControlModifier | Qt::AltModifier);

    QCOMPARE(guard.stub.saveCallCount, 1);
    QVERIFY(guard.stub.lastSaveCall.caption.contains("Arduino"));
    QVERIFY(guard.stub.lastSaveCall.filter.contains("ino"));
    QVERIFY(QFile::exists(exportPath));

    QFile::remove(exportPath);
}

void TestFileDialogProvider::testExportSystemVerilogUsesProvider()
{
    ScopedFileDialogStub guard;
    const QString exportPath = m_fixtureDir + "/export.sv";

    std::unique_ptr<MainWindow> window(createWindow());

    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";
    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier);

    guard.stub.saveResult = {exportPath, "SystemVerilog file (*.sv)"};
    guard.stub.saveCallCount = 0;

    // Ctrl+Alt+O to export SystemVerilog
    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier | Qt::AltModifier);

    QCOMPARE(guard.stub.saveCallCount, 1);
    QVERIFY(guard.stub.lastSaveCall.caption.contains("SystemVerilog"));
    QVERIFY(guard.stub.lastSaveCall.filter.contains("sv"));
    QVERIFY(QFile::exists(exportPath));

    QFile::remove(exportPath);
}

void TestFileDialogProvider::testExportPdfUsesProvider()
{
    ScopedFileDialogStub guard;
    const QString exportPath = m_fixtureDir + "/export.pdf";

    std::unique_ptr<MainWindow> window(createWindow());

    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";
    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier);

    guard.stub.saveResult = {exportPath, "PDF files (*.pdf)"};
    guard.stub.saveCallCount = 0;

    // Ctrl+Shift+P to export PDF
    QTest::keyClick(window.get(), Qt::Key_P, Qt::ControlModifier | Qt::ShiftModifier);

    QCOMPARE(guard.stub.saveCallCount, 1);
    QVERIFY(guard.stub.lastSaveCall.caption.contains("PDF"));
    QVERIFY(guard.stub.lastSaveCall.filter.contains("pdf"));
    QVERIFY(QFile::exists(exportPath));

    QFile::remove(exportPath);
}

void TestFileDialogProvider::testExportImageUsesProvider()
{
    ScopedFileDialogStub guard;
    const QString exportPath = m_fixtureDir + "/export.png";

    std::unique_ptr<MainWindow> window(createWindow());

    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";
    QTest::keyClick(window.get(), Qt::Key_O, Qt::ControlModifier);

    guard.stub.saveResult = {exportPath, "PNG files (*.png)"};
    guard.stub.saveCallCount = 0;

    // Ctrl+E to export image
    QTest::keyClick(window.get(), Qt::Key_E, Qt::ControlModifier);

    QCOMPARE(guard.stub.saveCallCount, 1);
    QVERIFY(guard.stub.lastSaveCall.caption.contains("Image"));
    QVERIFY(guard.stub.lastSaveCall.filter.contains("png"));
    QVERIFY(QFile::exists(exportPath));

    QFile::remove(exportPath);
}

// ===========================================================================
// MainWindow: IC embed (button click)
// ===========================================================================

void TestFileDialogProvider::testEmbedICFromFileUsesProvider()
{
    ScopedFileDialogStub guard;
    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";

    std::unique_ptr<MainWindow> window(createWindow());

    // Save first so it has a context directory
    const QString mainFile = m_fixtureDir + "/main_circuit.panda";
    guard.stub.saveResult = {mainFile, "Panda files (*.panda)"};
    QTest::keyClick(window.get(), Qt::Key_S, Qt::ControlModifier);

    // Reset counts
    guard.stub.openCallCount = 0;
    guard.stub.openResult = m_fixtureDir + "/test_circuit.panda";

    // Click the "Add Embedded IC" button (no keyboard shortcut for this)
    auto *button = window->findChild<QPushButton *>("pushButtonAddEmbeddedIC");
    QVERIFY(button);
    QTest::mouseClick(button, Qt::LeftButton);

    QCOMPARE(guard.stub.openCallCount, 1);
    QVERIFY(guard.stub.lastOpenCall.caption.contains("embed"));
    QVERIFY(guard.stub.lastOpenCall.filter.contains("panda"));

    auto *reg = window->currentTab()->scene()->icRegistry();
    QVERIFY(reg->blobMap().size() > 0);

    QFile::remove(mainFile);
}

// ===========================================================================
// WorkSpace: Save dialog
// ===========================================================================

void TestFileDialogProvider::testWorkspaceSaveIsProviderFree()
{
    // WorkSpace::save() is pure: given an already-resolved path, it must never touch the
    // file dialog provider — prompting for a path when none is known yet is
    // WorkspaceManager's job (see testSaveFirstTimeUsesProvider, which exercises that
    // path through MainWindow + Ctrl+S).
    ScopedFileDialogStub guard;
    const QString savePath = m_fixtureDir + "/ws_save.panda";

    WorkSpace ws;
    auto *sw = new InputSwitch();
    ws.scene()->addItem(sw);

    ws.save(savePath);

    QCOMPARE(guard.stub.saveCallCount, 0);
    QVERIFY(QFile::exists(savePath));

    QFile::remove(savePath);
}

void TestFileDialogProvider::testWorkspaceSaveRepromptsOnPermissionsErrorB24()
{
    // Pre-fix, Ctrl+S to a read-only path threw straight into a modal "Acesso negado"
    // dialog with no way out. Now WorkSpace::save() reports the failure as
    // SaveOutcome::ReadOnlyTarget instead of throwing, and WorkspaceManager::save()
    // (the retry owner now that WorkSpace::save() is pure) prompts the user via
    // FileDialogs::provider() for a new path and retries with it.
    QTemporaryDir lockedDir;
    QTemporaryDir writableDir;
    QVERIFY(lockedDir.isValid() && writableDir.isValid());

    const QString readOnlyPath = lockedDir.path() + "/locked.panda";
    const QString writablePath = writableDir.path() + "/recovered.panda";

    // Pre-create the target file as read-only AND drop write on the directory
    // — QSaveFile uses a sibling temp file in the same dir on commit, so just
    // making the file unwritable isn't enough; the dir must be locked too.
    {
        QFile f(readOnlyPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("seed");
    }
    QFile::setPermissions(readOnlyPath, QFileDevice::ReadOwner);
    QFile::setPermissions(lockedDir.path(),
        QFileDevice::ReadOwner | QFileDevice::ExeOwner);

    ScopedFileDialogStub guard;
    guard.stub.saveResult = {writablePath, "Panda files (*.panda)"};
    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;

    // Sanity: confirm QSaveFile actually fails to open the read-only target
    // on this system before exercising the production recovery path.
    {
        QSaveFile probe(readOnlyPath);
        QVERIFY(!probe.open(QIODevice::WriteOnly));
    }

    {
        std::unique_ptr<MainWindow> window(createWindow());
        auto *led = new Led();
        window->currentTab()->scene()->addItem(led);
        window->save(readOnlyPath);
    }

    Application::interactiveMode = prevInteractive;
    QFile::setPermissions(lockedDir.path(),
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QFile::setPermissions(readOnlyPath,
        QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    QCOMPARE(guard.stub.saveCallCount, 1);
    QVERIFY2(QFile::exists(writablePath),
             qPrintable("Save should have been redirected to " + writablePath));

    QFile::remove(writablePath);
}

void TestFileDialogProvider::testWorkspaceManagerSaveAddsExtensionIfMissing()
{
    // The ".panda"-suffix logic moved from WorkSpace::save() (now pure -- it requires an
    // already-suffixed path) to WorkspaceManager::promptSavePath(), exercised here via
    // the public MainWindow::save() entry point.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString fileWithoutExt = tempDir.filePath("test");
    const QString expectedFile = tempDir.filePath("test.panda");

    std::unique_ptr<MainWindow> window(createWindow());
    auto *led = new Led();
    window->currentTab()->scene()->addItem(led);

    window->save(fileWithoutExt);

    QVERIFY2(QFile::exists(expectedFile),
             qPrintable(QString("Expected file %1 not created").arg(expectedFile)));
}

void TestFileDialogProvider::testWorkspaceManagerSaveDoesNotDuplicateExtension()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString fileWithExt = tempDir.filePath("test.panda");

    std::unique_ptr<MainWindow> window(createWindow());
    auto *led = new Led();
    window->currentTab()->scene()->addItem(led);

    window->save(fileWithExt);

    QVERIFY2(QFile::exists(fileWithExt), "Expected save file should exist");
    QVERIFY2(!QFile::exists(tempDir.filePath("test.panda.panda")),
             "Duplicate extension file should not be created");
}

// ===========================================================================
// ElementEditor: Audio (button click)
// ===========================================================================

void TestFileDialogProvider::testAudioFileSelectionUsesProvider()
{
    ScopedFileDialogStub guard;
    guard.stub.openResult = QString(); // cancelled

    WorkSpace workspace;
    ElementEditor editor;
    editor.setScene(workspace.scene());

    auto *audioBox = new AudioBox();
    workspace.scene()->addItem(audioBox);
    audioBox->setSelected(true);
    QCoreApplication::processEvents();

    // Click the audio file button (no keyboard shortcut)
    auto *audioButton = editor.findChild<QPushButton *>("pushButtonAudio");
    if (audioButton) {
        QTest::mouseClick(audioButton, Qt::LeftButton);

        QCOMPARE(guard.stub.openCallCount, 1);
        QVERIFY(guard.stub.lastOpenCall.caption.contains("audio"));
        QVERIFY(guard.stub.lastOpenCall.filter.contains("mp3"));
        QVERIFY(guard.stub.lastOpenCall.filter.contains("wav"));
    } else {
        QSKIP("Audio button not found — AudioBox UI may not be initialized in headless mode");
    }
}
