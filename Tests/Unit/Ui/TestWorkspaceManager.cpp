// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestWorkspaceManager.h"

#include <memory>

#include <QFile>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTemporaryDir>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/MainWindowUI.h"
#include "App/UI/WorkspaceManager.h"
#include "Tests/Common/StubFileDialogProvider.h"
#include "Tests/Common/StubMainWindowHost.h"
#include "Tests/Common/TestUtils.h"

namespace {

// Builds a real, directly-constructible WorkspaceManager plus the collaborators it needs
// (a real ElementPalette so palette()->updateICList()/updateEmbeddedICList() calls have
// somewhere real to land, and a stub host for the rest). Mirrors the established
// TestElementPalette/TestSceneUiBinder fixture pattern.
struct WorkspaceManagerFixture {
    QMainWindow window;
    MainWindowUi ui;
    QTabWidget tabWidget;
    StubMainWindowHost host;
    std::unique_ptr<ElementPalette> palette;
    WorkspaceManager manager{&tabWidget, host};

    WorkspaceManagerFixture()
    {
        ui.setupUi(&window);
        palette = std::make_unique<ElementPalette>(&ui);
        host.widgetValue = &window;
        host.paletteValue = palette.get();
        // Mirrors MainWindow.cpp's real wiring: WorkspaceManager itself never connects this.
        QObject::connect(&tabWidget, &QTabWidget::currentChanged, &manager, &WorkspaceManager::onCurrentIndexChanged);
    }
};

// Pushes a real undo command onto ws's scene so its undo stack is no longer clean --
// the same technique TestWorkspace.cpp uses to dirty a workspace deterministically.
void dirtyUndoStack(WorkSpace *ws)
{
    auto *gate = new And();
    ws->scene()->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{gate}, ws->scene()));
}

} // namespace

void TestWorkspaceManager::testAccessorsAreSafeWithNoCurrentTab()
{
    WorkspaceManagerFixture f; // no createNewTab() -- m_currentTab is null

    QCOMPARE(f.manager.currentTabName(), QString());
    QVERIFY(f.manager.icListFile().filePath().isEmpty());
    QCOMPARE(f.manager.dolphinFileName(), QString());

    // No other observable effect when there's no tab -- the guard's whole contract is
    // "don't dereference a null m_currentTab," which not crashing here proves.
    f.manager.setDolphinFileName(QStringLiteral("x"));
}

void TestWorkspaceManager::testDolphinFileNameRoundTripsThroughCurrentTab()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    f.manager.setDolphinFileName(QStringLiteral("waveform.dolphin"));

    QCOMPARE(f.manager.dolphinFileName(), QStringLiteral("waveform.dolphin"));
}

void TestWorkspaceManager::testSaveWithNoExistingFileAndCancelledDialogIsNoOp()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = QString();

    f.manager.save();

    QCOMPARE(dialogStub.stub.saveCallCount, 1);
    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestWorkspaceManager::testSaveReadOnlyRetryCancelledReturnsEarly()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    QTemporaryDir readOnlyDir;
    QVERIFY(readOnlyDir.isValid());
    const QString readOnlyTarget = readOnlyDir.filePath("locked.panda");
    QVERIFY(QFile::setPermissions(readOnlyDir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = QString(); // cancel the "pick a writable location" re-prompt

    f.manager.save(readOnlyTarget);

    Application::interactiveMode = prevInteractive;
    QFile::setPermissions(readOnlyDir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QCOMPARE(dialogStub.stub.saveCallCount, 1);
    QVERIFY(!QFile::exists(readOnlyTarget));
    QCOMPARE(f.host.statusMessages.size(), 0);
#endif
}

void TestWorkspaceManager::testSaveRemovesAutosaveRecordOnSuccess()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("recovered.panda");

    // First save establishes the tab's real file path (currentFile() is no longer empty
    // afterward), so the second save below resolves the path directly instead of
    // re-triggering promptSavePath()'s "brand new + autosave-tracked" dialog prompt.
    f.manager.save(path);
    QVERIFY(QFile::exists(path));

    const QStringList prevAutosaves = Settings::autosaveFiles();
    Settings::setAutosaveFiles({path});

    f.manager.save(path);

    QVERIFY(!Settings::autosaveFiles().contains(path));

    Settings::setAutosaveFiles(prevAutosaves);
}

void TestWorkspaceManager::testCloseTabCancelledConfirmationReturnsFalse()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();
    dirtyUndoStack(f.manager.currentTab());

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);

    QVERIFY(!f.manager.closeTab(0));
    QCOMPARE(f.tabWidget.count(), 1);
}

void TestWorkspaceManager::testCloseTabDeclinedConfirmationClosesWithoutSaving()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();
    dirtyUndoStack(f.manager.currentTab());

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::No);

    QVERIFY(f.manager.closeTab(0));
    QCOMPARE(f.tabWidget.count(), 0);
}

void TestWorkspaceManager::testCloseTabSaveFailureThenCloseAnywayDiscards()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("project.panda");
    f.manager.save(path); // establish a real, already-saved file
    QVERIFY(QFile::exists(path));

    dirtyUndoStack(f.manager.currentTab());

    // Application::interactiveMode stays false (the test-suite default), so the re-save
    // below throws unconditionally on the now-unwritable directory instead of returning
    // WorkSpace::SaveOutcome::ReadOnlyTarget.
    QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        auto *box = qobject_cast<QMessageBox *>(w);
        if (!box) return false;
        if (box->text().contains(QStringLiteral("modified"))) {
            box->button(QMessageBox::Yes)->click();
        } else if (box->text().contains(QStringLiteral("Close tab anyway"))) {
            box->button(QMessageBox::Yes)->click();
        } else {
            box->accept(); // the QMessageBox::critical() error dialog in between
        }
        return true;
    });

    const bool closed = f.manager.closeTab(0);

    QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QVERIFY(closed);
    QCOMPARE(f.tabWidget.count(), 0);
#endif
}

void TestWorkspaceManager::testCloseTabSaveFailureThenCloseAnywayKeepsOpen()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("project.panda");
    f.manager.save(path);
    QVERIFY(QFile::exists(path));

    dirtyUndoStack(f.manager.currentTab());

    QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        auto *box = qobject_cast<QMessageBox *>(w);
        if (!box) return false;
        if (box->text().contains(QStringLiteral("modified"))) {
            box->button(QMessageBox::Yes)->click();
        } else if (box->text().contains(QStringLiteral("Close tab anyway"))) {
            box->button(QMessageBox::No)->click();
        } else {
            box->accept();
        }
        return true;
    });

    const bool closed = f.manager.closeTab(0);

    QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QVERIFY(!closed);
    QCOMPARE(f.tabWidget.count(), 1);
#endif
}

void TestWorkspaceManager::testConfirmSaveSingleOffersBasicButtons()
{
    WorkspaceManagerFixture f;
    QMessageBox::StandardButtons seenButtons;
    auto dismisser = TestUtils::AutoDismisser([&seenButtons](QWidget *w) {
        auto *box = qobject_cast<QMessageBox *>(w);
        if (!box) return false;
        seenButtons = box->standardButtons();
        box->button(QMessageBox::Yes)->click();
        return true;
    });

    const int result = f.manager.confirmSave(false);

    QCOMPARE(result, static_cast<int>(QMessageBox::Yes));
    QCOMPARE(seenButtons, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
}

void TestWorkspaceManager::testConfirmSaveMultipleOffersAllButtons()
{
    WorkspaceManagerFixture f;
    QMessageBox::StandardButtons seenButtons;
    auto dismisser = TestUtils::AutoDismisser([&seenButtons](QWidget *w) {
        auto *box = qobject_cast<QMessageBox *>(w);
        if (!box) return false;
        seenButtons = box->standardButtons();
        box->button(QMessageBox::YesToAll)->click();
        return true;
    });

    const int result = f.manager.confirmSave(true);

    QCOMPARE(result, static_cast<int>(QMessageBox::YesToAll));
    QCOMPARE(seenButtons, QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
}

void TestWorkspaceManager::testOpenICInTabIsNoOpWithNoCurrentTab()
{
    WorkspaceManagerFixture f;
    f.manager.openICInTab(QStringLiteral("blob"), 0, QByteArray());
    QCOMPARE(f.host.setICButtonsVisibleCallCount, 0);
    QCOMPARE(f.tabWidget.count(), 0);
}

void TestWorkspaceManager::testSaveFileCancelledDialogIsNoOp()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = QString();

    f.manager.saveFile();

    QCOMPARE(dialogStub.stub.saveCallCount, 1);
    QVERIFY(f.manager.currentFile().fileName().isEmpty());
}

void TestWorkspaceManager::testSaveFileAppendsExtensionWhenDialogOmitsIt()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString pathNoExt = dir.filePath("myproject");

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = pathNoExt;

    f.manager.saveFile();

    QVERIFY(QFile::exists(pathNoExt + ".panda"));
}

void TestWorkspaceManager::testSaveFileWarnsWhenTargetOpenInAnotherTab()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab(); // tab A

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("shared.panda");
    f.manager.save(path); // tab A now owns `path`

    f.manager.createNewTab(); // tab B, brand new, becomes current

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = path;

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    f.manager.saveFile();

    QVERIFY2(dismisser.dismissCount() >= 1, "the file-conflict warning must have appeared");
    // The warn-and-abort happened before save() was ever reached for tab B.
    QVERIFY(f.manager.currentFile().fileName().isEmpty());
}

void TestWorkspaceManager::testSaveFileAsCancelledIsNoOp()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = QString();

    f.manager.saveFileAs();

    QCOMPARE(dialogStub.stub.saveCallCount, 1);
    QVERIFY(f.manager.currentFile().fileName().isEmpty());
}

void TestWorkspaceManager::testReloadFileIsNoOpWithoutAnExistingFile()
{
    WorkspaceManagerFixture f; // no tab at all
    f.manager.reloadFile();
    QCOMPARE(f.tabWidget.count(), 0);
}

void TestWorkspaceManager::testCloseFilesClosesEveryTab()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();
    f.manager.createNewTab();
    QCOMPARE(f.tabWidget.count(), 2);

    QVERIFY(f.manager.closeFiles());
    QCOMPARE(f.tabWidget.count(), 0);
}

void TestWorkspaceManager::testCloseFilesStopsWhenATabCancels()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();
    f.manager.createNewTab();

    // closeFiles() closes from last to first; dirty the current (last) tab so its
    // close attempt prompts for confirmation.
    dirtyUndoStack(f.manager.currentTab());

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);

    QVERIFY(!f.manager.closeFiles());
    QCOMPARE(f.tabWidget.count(), 2);
}

void TestWorkspaceManager::testLoadAutosaveFilesRemovesMissingFile()
{
    WorkspaceManagerFixture f;
    const QStringList prevAutosaves = Settings::autosaveFiles();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString missingPath = dir.filePath("gone.panda"); // never actually created

    Settings::setAutosaveFiles({missingPath});

    f.manager.loadAutosaveFiles();

    QVERIFY(!Settings::autosaveFiles().contains(missingPath));
    QCOMPARE(f.tabWidget.count(), 0);

    Settings::setAutosaveFiles(prevAutosaves);
}

void TestWorkspaceManager::testLoadAutosaveFilesRemovesCorruptedFile()
{
    WorkspaceManagerFixture f;
    const QStringList prevAutosaves = Settings::autosaveFiles();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString corruptPath = dir.filePath("corrupt.panda");
    QFile file(corruptPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("not a real panda file");
    file.close();

    Settings::setAutosaveFiles({corruptPath});

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    f.manager.loadAutosaveFiles();

    Application::interactiveMode = prevInteractive;

    QVERIFY(!Settings::autosaveFiles().contains(corruptPath));
    QCOMPARE(f.tabWidget.count(), 0);
    QVERIFY2(dismisser.dismissCount() >= 1, "the corrupted-autosave error dialog must have appeared");

    Settings::setAutosaveFiles(prevAutosaves);
}

void TestWorkspaceManager::testSetCurrentFileFallsBackToCurrentTabWhenNotCalledAsSlot()
{
    WorkspaceManagerFixture f;
    f.manager.createNewTab();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("renamed.panda");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    // Calling setCurrentFile() as a plain function call (not via a real signal emission)
    // makes QObject::sender() return null, exercising the "fall back to m_currentTab"
    // branch instead of the normal signal-driven path.
    f.manager.setCurrentFile(QFileInfo(path));

    QCOMPARE(f.tabWidget.tabText(0), QStringLiteral("renamed.panda"));
}

void TestWorkspaceManager::testSetCurrentFileIsNoOpWithNoSenderAndNoCurrentTab()
{
    WorkspaceManagerFixture f; // no tab at all
    f.manager.setCurrentFile(QFileInfo());
    QCOMPARE(f.tabWidget.count(), 0);
}

void TestWorkspaceManager::testSetCurrentFileIsNoOpWhenSenderTabNotInTabWidget()
{
    WorkspaceManagerFixture f;
    WorkSpace orphan; // never added to f.tabWidget
    connect(&orphan, &WorkSpace::fileChanged, &f.manager, &WorkspaceManager::setCurrentFile);

    emit orphan.fileChanged(QFileInfo());

    QCOMPARE(f.tabWidget.count(), 0);
}
