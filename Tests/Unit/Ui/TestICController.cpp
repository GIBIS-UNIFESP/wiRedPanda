// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestICController.h"

#include <memory>

#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/IC.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/ICController.h"
#include "App/UI/MainWindowUI.h"
#include "Tests/Common/ICTestHelpers.h"
#include "Tests/Common/StubFileDialogProvider.h"
#include "Tests/Common/StubMainWindowHost.h"
#include "Tests/Common/TestUtils.h"

namespace {

struct ICControllerFixture {
    QMainWindow window;
    MainWindowUi ui;
    std::unique_ptr<ElementPalette> palette;
    StubMainWindowHost host;
    WorkSpace tab;

    ICControllerFixture()
    {
        ui.setupUi(&window);
        palette = std::make_unique<ElementPalette>(&ui);
        host.widgetValue = &window;
        host.paletteValue = palette.get();
        host.currentTabValue = &tab;
    }
};

// ICRegistry::embedICsByFile()/extractToFile() accept this as real IC file content.
using ICTestHelpers::minimalPandaBytes;

// Saves `tab` to a real file under `dir`, giving its scene a non-empty contextDir so
// ensureProjectSaved() returns true without needing to show any dialog.
void saveProject(WorkSpace &tab, StubMainWindowHost &host, const QTemporaryDir &dir)
{
    tab.save(dir.filePath("project.panda"));
    host.currentDirValue = QDir(dir.path());
}

// Selects an IC embedded under `blobName`, with the blob already registered in the scene's
// ICRegistry.
IC *selectEmbeddedIC(WorkSpace &tab, const QString &blobName, const QByteArray &blobBytes)
{
    auto *ic = new IC();
    tab.scene()->addItem(ic);
    ICTestHelpers::embedIC(ic, blobBytes, blobName, tab.scene()->contextDir(), tab.scene()->icRegistry());
    ic->setSelected(true);
    return ic;
}

} // namespace

// Members (not free functions) so IC's "friend class TestICController;" grant covers direct
// access to IC::m_file below.
IC *TestICController::selectBareIC(WorkSpace &tab)
{
    auto *ic = new IC();
    tab.scene()->addItem(ic);
    ic->setSelected(true);
    return ic;
}

IC *TestICController::selectFileBackedIC(WorkSpace &tab, const QString &fileName)
{
    // Mirrors ICLoader::loadFile()'s real behavior (ic.m_file = fileInfo.absoluteFilePath()),
    // not the *deserialization* path's bare-filename form (IC::load() strips the directory).
    // ICController::embedICByFile()/makeSelfContained() build an absolute path themselves
    // before calling ICRegistry::findICsByFile(), which compares QFileInfo equality -- a bare
    // relative m_file would never match, exactly like a freshly-dragged-in file-backed IC
    // (the real, common case these two callers exist for) never would either.
    auto *ic = new IC();
    ic->m_file = QDir(tab.scene()->contextDir()).absoluteFilePath(fileName);
    tab.scene()->addItem(ic);
    ic->setSelected(true);
    return ic;
}

void TestICController::testEmbedSelectedICNoOpWithNoCurrentTab()
{
    ICControllerFixture f;
    f.host.currentTabValue = nullptr;
    ICController controller(f.host);

    controller.embedSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedSelectedICNoOpWithEmptySelection()
{
    ICControllerFixture f;
    ICController controller(f.host);

    controller.embedSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedSelectedICNoOpWithNonICSelected()
{
    ICControllerFixture f;
    auto *gate = new And();
    f.tab.scene()->addItem(gate);
    gate->setSelected(true);
    ICController controller(f.host);

    controller.embedSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedSelectedICNoOpWithEmptyFileName()
{
    ICControllerFixture f;
    selectBareIC(f.tab); // m_file left empty
    ICController controller(f.host);

    controller.embedSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedSelectedICNoOpWhenProjectNotSavedAndDeclined()
{
    ICControllerFixture f;
    selectFileBackedIC(f.tab, QStringLiteral("dep.panda")); // scene never saved -> empty contextDir
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);

    controller.embedSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedSelectedICWarnsWhenFileMissing()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);
    selectFileBackedIC(f.tab, QStringLiteral("missing.panda")); // never written to disk
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    controller.embedSelectedIC();

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"could not read IC file\" warning must have appeared");
    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedSelectedICBlobNameCollisionCancelled()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    QFile depFile(dir.filePath("dep.panda"));
    QVERIFY(depFile.open(QIODevice::WriteOnly));
    depFile.write(minimalPandaBytes());
    depFile.close();

    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("dep"), minimalPandaBytes());
    selectFileBackedIC(f.tab, QStringLiteral("dep.panda"));
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        auto *dialog = qobject_cast<QInputDialog *>(w);
        if (!dialog) return false;
        dialog->reject();
        return true;
    });

    controller.embedSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedSelectedICSucceeds()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    QFile depFile(dir.filePath("dep.panda"));
    QVERIFY(depFile.open(QIODevice::WriteOnly));
    depFile.write(minimalPandaBytes());
    depFile.close();

    selectFileBackedIC(f.tab, QStringLiteral("dep.panda"));
    ICController controller(f.host);

    controller.embedSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 1);
    QVERIFY(f.tab.scene()->icRegistry()->hasBlob(QStringLiteral("dep")));
}

void TestICController::testExtractSelectedICNoOpWhenNotEmbedded()
{
    ICControllerFixture f;
    ICController controller(f.host);

    controller.extractSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testExtractSelectedICNoOpWhenProjectNotSavedAndDeclined()
{
    ICControllerFixture f; // scene never saved
    selectEmbeddedIC(f.tab, QStringLiteral("myblob"), minimalPandaBytes());
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);

    controller.extractSelectedIC();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testExtractSelectedICNoOpWhenDialogCancelled()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);
    selectEmbeddedIC(f.tab, QStringLiteral("myblob"), minimalPandaBytes());
    ICController controller(f.host);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = QString();

    controller.extractSelectedIC();

    QCOMPARE(dialogStub.stub.saveCallCount, 1);
    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testExtractSelectedICSucceeds()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);
    selectEmbeddedIC(f.tab, QStringLiteral("myblob"), minimalPandaBytes());
    ICController controller(f.host);

    const QString pathNoExt = dir.filePath("extracted");
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = pathNoExt;

    controller.extractSelectedIC();

    QVERIFY2(QFile::exists(pathNoExt + ".panda"), "the .panda extension must have been appended");
    QCOMPARE(f.host.statusMessages.size(), 1);
}

void TestICController::testEmbedICByFileNoOpWithNoCurrentTab()
{
    ICControllerFixture f;
    f.host.currentTabValue = nullptr;
    ICController controller(f.host);

    controller.embedICByFile(QStringLiteral("dep.panda"));

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedICByFileNoOpWhenProjectNotSavedAndDeclined()
{
    ICControllerFixture f; // scene never saved
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);

    controller.embedICByFile(QStringLiteral("dep.panda"));

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedICByFileWarnsWhenFileMissing()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    controller.embedICByFile(QStringLiteral("missing.panda")); // never written to disk

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"could not read IC file\" warning must have appeared");
    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedICByFileBlobNameCollisionCancelled()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    QFile depFile(dir.filePath("dep.panda"));
    QVERIFY(depFile.open(QIODevice::WriteOnly));
    depFile.write(minimalPandaBytes());
    depFile.close();

    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("dep"), minimalPandaBytes());
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        auto *dialog = qobject_cast<QInputDialog *>(w);
        if (!dialog) return false;
        dialog->reject();
        return true;
    });

    controller.embedICByFile(QStringLiteral("dep.panda"));

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testEmbedICByFileSucceeds()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    QFile depFile(dir.filePath("dep.panda"));
    QVERIFY(depFile.open(QIODevice::WriteOnly));
    depFile.write(minimalPandaBytes());
    depFile.close();

    ICController controller(f.host);
    controller.embedICByFile(QStringLiteral("dep.panda"));

    QCOMPARE(f.host.statusMessages.size(), 1);
    QVERIFY(f.tab.scene()->icRegistry()->hasBlob(QStringLiteral("dep")));
}

void TestICController::testExtractICByBlobNameNoOpWhenBlobUnknown()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    ICController controller(f.host);
    controller.extractICByBlobName(QStringLiteral("neverRegistered"));

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testExtractICByBlobNameNoOpWhenProjectNotSavedAndDeclined()
{
    ICControllerFixture f; // scene never saved
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);

    controller.extractICByBlobName(QStringLiteral("anyblob"));

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testExtractICByBlobNameNoOpWhenDialogCancelled()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);
    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("myblob"), minimalPandaBytes());

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = QString();

    controller.extractICByBlobName(QStringLiteral("myblob"));

    QCOMPARE(dialogStub.stub.saveCallCount, 1);
    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testExtractICByBlobNameSucceeds()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);
    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("myblob"), minimalPandaBytes());

    ICController controller(f.host);
    const QString pathNoExt = dir.filePath("extracted2");
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = pathNoExt;

    controller.extractICByBlobName(QStringLiteral("myblob"));

    QVERIFY2(QFile::exists(pathNoExt + ".panda"), "the .panda extension must have been appended");
    QCOMPARE(f.host.statusMessages.size(), 1);
}

void TestICController::testMakeSelfContainedNoOpWithNoFileBackedICs()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    ICController controller(f.host);
    controller.makeSelfContained();

    QCOMPARE(f.host.statusMessages.size(), 1);
    QCOMPARE(f.host.statusMessages.constFirst().message, QStringLiteral("No file-based ICs to embed."));
}

void TestICController::testMakeSelfContainedNoOpWhenProjectNotSavedAndDeclined()
{
    ICControllerFixture f; // scene never saved
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);

    controller.makeSelfContained();

    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestICController::testMakeSelfContainedSkipsNonFileBackedElements()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    QFile depFile(dir.filePath("dep.panda"));
    QVERIFY(depFile.open(QIODevice::WriteOnly));
    depFile.write(minimalPandaBytes());
    depFile.close();

    // A non-IC element and an already-embedded IC must both be skipped by the file-backed
    // scan -- only the file-backed IC below should end up in uniqueFiles.
    f.tab.scene()->addItem(new And());
    selectEmbeddedIC(f.tab, QStringLiteral("alreadyEmbedded"), minimalPandaBytes());
    selectFileBackedIC(f.tab, QStringLiteral("dep.panda"));

    ICController controller(f.host);
    controller.makeSelfContained();

    QCOMPARE(f.host.statusMessages.size(), 1);
    QVERIFY2(f.host.statusMessages.constFirst().message.contains(QStringLiteral("self-contained")),
              "the one file-backed IC must have embedded successfully, ignoring the other elements");
    QVERIFY(f.tab.scene()->icRegistry()->hasBlob(QStringLiteral("dep")));
}

void TestICController::testMakeSelfContainedWarnsWhenFileMissing()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    selectFileBackedIC(f.tab, QStringLiteral("missing.panda")); // never written to disk
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    controller.makeSelfContained();

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"could not read IC file\" warning must have appeared");
    QCOMPARE(f.host.statusMessages.size(), 0); // completed=false and totalEmbedded==0 -> stays quiet
}

void TestICController::testMakeSelfContainedReportsPartialCompletionOnCancelledCollision()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    QFile dep1File(dir.filePath("dep1.panda"));
    QVERIFY(dep1File.open(QIODevice::WriteOnly));
    dep1File.write(minimalPandaBytes());
    dep1File.close();

    QFile dep2File(dir.filePath("dep2.panda"));
    QVERIFY(dep2File.open(QIODevice::WriteOnly));
    dep2File.write(minimalPandaBytes());
    dep2File.close();

    // Pre-register a blob colliding with dep2's base name so resolveUniqueBlobName() must
    // prompt for it specifically; cancelling makes the loop break with dep2 unembedded while
    // dep1 (whichever of the two is processed without a collision) succeeds -- covering the
    // "some embedded, some remain" partial-completion message.
    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("dep2"), minimalPandaBytes());
    selectFileBackedIC(f.tab, QStringLiteral("dep1.panda"));
    auto *ic2 = new IC();
    ic2->m_file = QDir(f.tab.scene()->contextDir()).absoluteFilePath(QStringLiteral("dep2.panda"));
    f.tab.scene()->addItem(ic2);

    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        auto *dialog = qobject_cast<QInputDialog *>(w);
        if (!dialog) return false;
        dialog->reject();
        return true;
    });

    controller.makeSelfContained();

    QVERIFY2(f.tab.scene()->icRegistry()->hasBlob(QStringLiteral("dep1")),
              "dep1 (no collision) must have embedded successfully before the loop broke on dep2");
    QCOMPARE(f.host.statusMessages.size(), 1);
    QVERIFY2(f.host.statusMessages.constFirst().message.contains(QStringLiteral("remain")),
              "a cancelled collision prompt must produce the partial-completion message");
}

void TestICController::testMakeSelfContainedSucceeds()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    QFile depFile(dir.filePath("dep.panda"));
    QVERIFY(depFile.open(QIODevice::WriteOnly));
    depFile.write(minimalPandaBytes());
    depFile.close();

    selectFileBackedIC(f.tab, QStringLiteral("dep.panda"));

    ICController controller(f.host);
    controller.makeSelfContained();

    QCOMPARE(f.host.statusMessages.size(), 1);
    QVERIFY2(f.host.statusMessages.constFirst().message.contains(QStringLiteral("self-contained")),
              "every file-based IC embedded successfully must report full completion");
    QVERIFY(f.tab.scene()->icRegistry()->hasBlob(QStringLiteral("dep")));
}

void TestICController::testAddEmbeddedICFromFileNoOpWithNoCurrentTab()
{
    ICControllerFixture f;
    f.host.currentTabValue = nullptr;
    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;

    controller.addEmbeddedICFromFile();

    QCOMPARE(dialogStub.stub.openCallCount, 0);
}

void TestICController::testAddEmbeddedICFromFileNoOpWhenDialogCancelled()
{
    ICControllerFixture f;
    ICController controller(f.host);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = QString();

    controller.addEmbeddedICFromFile();

    QCOMPARE(dialogStub.stub.openCallCount, 1);
}

void TestICController::testAddEmbeddedICFromFileWarnsWhenFileUnreadable()
{
    ICControllerFixture f;
    ICController controller(f.host);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = QStringLiteral("/nonexistent/directory/missing.panda");

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    controller.addEmbeddedICFromFile();

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"could not read file\" warning must have appeared");
}

void TestICController::testAddEmbeddedICFromFileSucceeds()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile srcFile(dir.filePath("newblob.panda"));
    QVERIFY(srcFile.open(QIODevice::WriteOnly));
    srcFile.write(minimalPandaBytes());
    srcFile.close();

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = dir.filePath("newblob.panda");

    controller.addEmbeddedICFromFile();

    QVERIFY(f.tab.scene()->icRegistry()->hasBlob(QStringLiteral("newblob")));
}

void TestICController::testAddEmbeddedICFromFileBlobNameCollisionCancelled()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile srcFile(dir.filePath("newblob.panda"));
    QVERIFY(srcFile.open(QIODevice::WriteOnly));
    srcFile.write(minimalPandaBytes());
    srcFile.close();

    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("newblob"), minimalPandaBytes());

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = dir.filePath("newblob.panda");

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        auto *dialog = qobject_cast<QInputDialog *>(w);
        if (!dialog) return false;
        dialog->reject();
        return true;
    });

    controller.addEmbeddedICFromFile();

    QVERIFY2(dismisser.dismissCount() >= 1, "the name-collision prompt must have appeared");
}

void TestICController::testResolveUniqueBlobNameCollisionCancelled()
{
    // Dedicated pin for resolveUniqueBlobName()'s own re-check-after-user-edit branch: the
    // user is offered a suggested (already-unique) name but types back in one that *also*
    // collides, which must be silently re-uniquified rather than accepted as-is.
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile srcFile(dir.filePath("newblob.panda"));
    QVERIFY(srcFile.open(QIODevice::WriteOnly));
    srcFile.write(minimalPandaBytes());
    srcFile.close();

    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("newblob"), minimalPandaBytes());
    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("newblob2"), minimalPandaBytes());

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = dir.filePath("newblob.panda");

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        auto *dialog = qobject_cast<QInputDialog *>(w);
        if (!dialog) return false;
        // Type back in a name that *also* collides (newblob2), instead of accepting the
        // dialog's own already-unique suggestion.
        dialog->setTextValue(QStringLiteral("newblob2"));
        dialog->accept();
        return true;
    });

    controller.addEmbeddedICFromFile();

    QVERIFY(f.tab.scene()->icRegistry()->hasBlob(QStringLiteral("newblob2")));
    // A third, freshly-uniquified name must have been registered instead of colliding.
    const auto blobNames = f.tab.scene()->icRegistry()->blobMap().keys();
    QVERIFY2(blobNames.size() == 3, "the re-collision must have been silently re-resolved to a third name");
}

void TestICController::testRemoveEmbeddedICNoOpWithNoCurrentTab()
{
    ICControllerFixture f;
    f.host.currentTabValue = nullptr;
    ICController controller(f.host);

    controller.removeEmbeddedIC(QStringLiteral("anyblob"));
    // No crash is the only contract when there's no current tab to act on.
}

void TestICController::testRemoveEmbeddedICSucceeds()
{
    ICControllerFixture f;
    f.tab.scene()->icRegistry()->setBlob(QStringLiteral("myblob"), minimalPandaBytes());
    selectEmbeddedIC(f.tab, QStringLiteral("myblob"), minimalPandaBytes());
    ICController controller(f.host);

    controller.removeEmbeddedIC(QStringLiteral("myblob"));

    QVERIFY(!f.tab.scene()->icRegistry()->hasBlob(QStringLiteral("myblob")));
}

void TestICController::testAddICFromFileNoOpWithNoCurrentTab()
{
    ICControllerFixture f;
    f.host.currentTabValue = nullptr;
    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;

    controller.addICFromFile();

    QCOMPARE(dialogStub.stub.openCallCount, 0);
}

void TestICController::testAddICFromFileNoOpWhenProjectNotSavedAndDeclined()
{
    ICControllerFixture f; // scene never saved
    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);

    controller.addICFromFile();

    QCOMPARE(dialogStub.stub.openCallCount, 0);
}

void TestICController::testAddICFromFileNoOpWhenSaveRequestedButStillUnsaved()
{
    // Answering "Save" (rather than Cancel) to the "project needs saving" prompt calls
    // MainWindowHost::requestSave() -- StubMainWindowHost's implementation just counts the
    // call rather than performing a real save, so the scene's contextDir() stays empty and
    // ensureProjectSaved() must still correctly report "not saved" afterward.
    ICControllerFixture f; // scene never saved
    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Save);

    controller.addICFromFile();

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"Save required\" prompt must have appeared");
    QCOMPARE(f.host.requestSaveCallCount, 1);
    QCOMPARE(dialogStub.stub.openCallCount, 0);
}

void TestICController::testAddICFromFileNoOpWhenDialogCancelled()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    saveProject(f.tab, f.host, dir);

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = QString();

    controller.addICFromFile();

    QCOMPARE(dialogStub.stub.openCallCount, 1);
}

void TestICController::testAddICFromFileCopiesFile()
{
    ICControllerFixture f;
    QTemporaryDir projectDir;
    QVERIFY(projectDir.isValid());
    saveProject(f.tab, f.host, projectDir);

    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    QFile srcFile(sourceDir.filePath("import.panda"));
    QVERIFY(srcFile.open(QIODevice::WriteOnly));
    srcFile.write(minimalPandaBytes());
    srcFile.close();

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = sourceDir.filePath("import.panda");

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox(); // the "will be copied" info dialog

    controller.addICFromFile();

    QVERIFY2(QFile::exists(projectDir.filePath("import.panda")), "the file must have been copied into the project directory");
}

void TestICController::testAddICFromFileConflictKeepExisting()
{
    ICControllerFixture f;
    QTemporaryDir projectDir;
    QVERIFY(projectDir.isValid());
    saveProject(f.tab, f.host, projectDir);

    // A different-content file already occupies the destination name.
    QFile existingFile(projectDir.filePath("import.panda"));
    QVERIFY(existingFile.open(QIODevice::WriteOnly));
    existingFile.write("existing content, deliberately different");
    existingFile.close();

    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    QFile srcFile(sourceDir.filePath("import.panda"));
    QVERIFY(srcFile.open(QIODevice::WriteOnly));
    srcFile.write(minimalPandaBytes());
    srcFile.close();

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = sourceDir.filePath("import.panda");

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        if (auto *box = qobject_cast<QMessageBox *>(w)) {
            if (box->text().contains(QStringLiteral("Selected files"))) {
                box->accept();
                return true;
            }
            for (auto *btn : box->buttons()) {
                if (btn->text().remove('&') == QStringLiteral("Keep Existing")) {
                    btn->click();
                    return true;
                }
            }
        }
        return false;
    });

    controller.addICFromFile();

    QFile keptFile(projectDir.filePath("import.panda"));
    QVERIFY(keptFile.open(QIODevice::ReadOnly));
    QCOMPARE(keptFile.readAll(), QByteArray("existing content, deliberately different"));
}

void TestICController::testAddICFromFileConflictReplace()
{
    ICControllerFixture f;
    QTemporaryDir projectDir;
    QVERIFY(projectDir.isValid());
    saveProject(f.tab, f.host, projectDir);

    QFile existingFile(projectDir.filePath("import.panda"));
    QVERIFY(existingFile.open(QIODevice::WriteOnly));
    existingFile.write("existing content, deliberately different");
    existingFile.close();

    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    QFile srcFile(sourceDir.filePath("import.panda"));
    QVERIFY(srcFile.open(QIODevice::WriteOnly));
    srcFile.write(minimalPandaBytes());
    srcFile.close();

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = sourceDir.filePath("import.panda");

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        if (auto *box = qobject_cast<QMessageBox *>(w)) {
            if (box->text().contains(QStringLiteral("Selected files"))) {
                box->accept();
                return true;
            }
            for (auto *btn : box->buttons()) {
                if (btn->text().remove('&') == QStringLiteral("Replace")) {
                    btn->click();
                    return true;
                }
            }
        }
        return false;
    });

    controller.addICFromFile();

    QFile replacedFile(projectDir.filePath("import.panda"));
    QVERIFY(replacedFile.open(QIODevice::ReadOnly));
    QCOMPARE(replacedFile.readAll(), minimalPandaBytes());
}

void TestICController::testAddICFromFileConflictCancelled()
{
    ICControllerFixture f;
    QTemporaryDir projectDir;
    QVERIFY(projectDir.isValid());
    saveProject(f.tab, f.host, projectDir);

    const QByteArray existingContent = "existing content, deliberately different";
    QFile existingFile(projectDir.filePath("import.panda"));
    QVERIFY(existingFile.open(QIODevice::WriteOnly));
    existingFile.write(existingContent);
    existingFile.close();

    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    QFile srcFile(sourceDir.filePath("import.panda"));
    QVERIFY(srcFile.open(QIODevice::WriteOnly));
    srcFile.write(minimalPandaBytes());
    srcFile.close();

    ICController controller(f.host);
    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = sourceDir.filePath("import.panda");

    auto dismisser = TestUtils::AutoDismisser([](QWidget *w) {
        if (auto *box = qobject_cast<QMessageBox *>(w)) {
            if (box->text().contains(QStringLiteral("Selected files"))) {
                box->accept();
                return true;
            }
            if (auto *cancelBtn = box->button(QMessageBox::Cancel)) {
                cancelBtn->click();
                return true;
            }
        }
        return false;
    });

    controller.addICFromFile();

    QFile untouchedFile(projectDir.filePath("import.panda"));
    QVERIFY(untouchedFile.open(QIODevice::ReadOnly));
    QCOMPARE(untouchedFile.readAll(), existingContent);
    QCOMPARE(f.host.statusMessages.size(), 0); // updateICList() line never reached
}

void TestICController::testShowRemoveICHintShowsInfo()
{
    ICControllerFixture f;
    ICController controller(f.host);

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    controller.showRemoveICHint();

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"Drag here to remove\" info dialog must have appeared");
}

void TestICController::testRemoveICFileNoOpWithNoCurrentTab()
{
    ICControllerFixture f;
    f.host.currentTabValue = nullptr;
    ICController controller(f.host);

    controller.removeICFile(QStringLiteral("anything.panda"));

    QCOMPARE(f.host.requestSaveCallCount, 0);
}

void TestICController::testRemoveICFileThrowsWhenTrashFails()
{
#if defined(Q_OS_MACOS)
    // macOS has no /etc/hostname at all (confirmed: it configures the hostname via scutil/
    // Open Directory instead of a plain file) -- unlike the Linux case below, there's no
    // verified-safe substitute file to point at without live access to the exact runner image,
    // so skip rather than guess at another system path and risk the same failure again.
    QSKIP("No verified-safe root-only file to target on macOS (see comment)");
#elif defined(Q_OS_WIN)
    // Windows has no /etc directory at all, so this resolves to a nonexistent path instead
    // of a real permission-denied one -- there's no equivalent "definitely inaccessible to
    // this process" file without real ACL manipulation (see the QSKIP precedent already
    // established in TestArduino.cpp for the same underlying POSIX-permissions-vs-ACLs gap).
    QSKIP("QFile::moveToTrash() failure can't be simulated via a fixed path on Windows (uses ACLs, not Unix permission bits)");
#else
    // Locking the containing directory (chmod'ing out write access) does NOT make
    // QFile::moveToTrash() fail in this sandbox -- confirmed empirically: its XDG trash
    // implementation still succeeds regardless, and neither does stripping the file's own
    // permission bits (still the file's owner). What does reliably fail is a file this
    // process genuinely cannot write to at all: a real root-owned file in a root-only-writable
    // directory. /etc/hostname (not /usr/bin/ls or similar) specifically, so that even if this
    // assumption were ever wrong in some other environment, the blast radius is a harmless,
    // purely informational file rather than a core system binary.
    ICControllerFixture f;
    f.host.currentDirValue = QDir(QStringLiteral("/etc"));

    ICController controller(f.host);
    bool threw = false;
    try {
        controller.removeICFile(QStringLiteral("hostname"));
    } catch (const std::exception &) {
        threw = true;
    }

    QVERIFY2(threw, "a failed moveToTrash() must throw rather than silently proceed");
#endif
}

void TestICController::testRemoveICFileDeletesInstancesAndSaves()
{
    ICControllerFixture f;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    f.host.currentDirValue = QDir(dir.path());

    QFile icFile(dir.filePath("dep.panda"));
    QVERIFY(icFile.open(QIODevice::WriteOnly));
    icFile.write(minimalPandaBytes());
    icFile.close();

    selectFileBackedIC(f.tab, QStringLiteral("dep.panda"));
    auto *gate = new And(); // non-IC element: must be skipped by the IC-only scan, not deleted
    f.tab.scene()->addItem(gate);

    ICController controller(f.host);
    controller.removeICFile(QStringLiteral("dep.panda"));

    QCOMPARE(f.tab.scene()->elements().size(), 1);
    QVERIFY(f.tab.scene()->elements().constFirst() == gate);
    QCOMPARE(f.host.requestSaveCallCount, 1);
}
