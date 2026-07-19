// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasInlineIC.h"

#include <QFile>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/QuickShell/Chrome/QuickWorkspaceManager.h"
#include "Tests/Common/StubFileDialogProvider.h"

namespace {

IC *placeInlineTestIC(CanvasItem &canvas, const QString &fixtureDir, const QString &blobName,
                    const QPointF &pos = {100, 100})
{
    QFile file(fixtureDir + "/test_circuit.panda");
    if (!file.open(QIODevice::ReadOnly)) {
        return nullptr;
    }
    const QByteArray blob = file.readAll();

    auto *reg = canvas.icRegistry();
    auto *ic = new IC();
    reg->setBlob(blobName, blob);
    ic->setBlobName(blobName);
    ic->loadFromBlob(blob, fixtureDir);
    ic->setPos(pos);
    canvas.receiveCommand(new CanvasAddItemsCommand({ic}, &canvas));
    return ic;
}

} // namespace

void TestCanvasInlineIC::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    m_fixtureDir = m_tempDir.path();

    QuickWorkSpace ws;
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(ws.save(m_fixtureDir + "/test_circuit.panda") == QuickWorkSpace::SaveOutcome::Saved, "fixture write");
}

void TestCanvasInlineIC::testInlineTabDeduplication()
{
    QuickWorkspaceManager wsManager;
    wsManager.newTab();
    auto *canvas = wsManager.currentTab()->canvas();
    auto *ic = placeInlineTestIC(*canvas, m_fixtureDir, "dedup_test");
    const QByteArray blob = canvas->icRegistry()->blob("dedup_test");

    const int initialTabs = wsManager.count();

    wsManager.openICInTab("dedup_test", ic->id(), blob);
    QCOMPARE(wsManager.count(), initialTabs + 1);

    // Reopening the same blob switches to the existing inline tab instead of duplicating it.
    wsManager.setCurrentIndex(0);
    wsManager.openICInTab("dedup_test", ic->id(), blob);
    QCOMPARE(wsManager.count(), initialTabs + 1);
}

void TestCanvasInlineIC::testInlineTabTitleUpdatesAfterBlobRename()
{
    QuickWorkspaceManager wsManager;
    wsManager.newTab();
    auto *canvas = wsManager.currentTab()->canvas();
    auto *ic = placeInlineTestIC(*canvas, m_fixtureDir, "rename_tab_test");
    const QByteArray blob = canvas->icRegistry()->blob("rename_tab_test");

    wsManager.openICInTab("rename_tab_test", ic->id(), blob);
    auto *inlineTab = wsManager.currentTab();
    QCOMPARE(wsManager.tabTitle(inlineTab), QString("[rename_tab_test]"));

    // Rename the blob from the parent canvas (the inline tab stays open throughout) --
    // CanvasICRegistry::blobRenamed, connected in createNewTab(), must retitle it live.
    canvas->icRegistry()->renameBlob("rename_tab_test", "renamed_tab_test");

    QCOMPARE(wsManager.tabTitle(inlineTab), QString("[renamed_tab_test]"));
    QCOMPARE(inlineTab->inlineBlobName(), QString("renamed_tab_test"));
}

void TestCanvasInlineIC::testEmbeddedICSaveReloadRoundTrip()
{
    const QString savePath = m_fixtureDir + "/save_reload_test.panda";
    QFile::remove(savePath);

    {
        QuickAppController controller;
        controller.newTab();
        auto *canvas = controller.currentTab()->canvas();
        placeInlineTestIC(*canvas, m_fixtureDir, "save_reload");

        controller.saveCurrentTabAs(savePath);
    }

    {
        QuickAppController controller2;
        controller2.newTab();
        controller2.openRecentFile(savePath);
        auto *canvas2 = controller2.currentTab()->canvas();

        QVERIFY(canvas2->icRegistry()->hasBlob("save_reload"));
        int found = 0;
        for (auto *elm : canvas2->elements()) {
            if (elm->isEmbedded() && elm->blobName() == "save_reload") {
                ++found;
                QVERIFY(elm->inputSize() > 0);
                QVERIFY(elm->outputSize() > 0);
            }
        }
        QCOMPARE(found, 1);
    }

    QFile::remove(savePath);
}

void TestCanvasInlineIC::testInlineICSaveNoFileDialog()
{
    // Ctrl+S-equivalent (saveFile()) on an inline IC tab must not open a file dialog -- it
    // always writes back to its own blob via QuickWorkSpace::save(QString())'s inline branch.
    QuickWorkspaceManager wsManager;
    wsManager.newTab();
    auto *canvas = wsManager.currentTab()->canvas();
    auto *ic = placeInlineTestIC(*canvas, m_fixtureDir, "no_dialog_test");
    const QByteArray blob = canvas->icRegistry()->blob("no_dialog_test");

    wsManager.openICInTab("no_dialog_test", ic->id(), blob);
    QVERIFY(wsManager.currentTab()->isInlineIC());

    wsManager.currentTab()->canvas()->addItem(ElementFactory::buildElement(ElementType::And));

    ScopedFileDialogStub guard;
    wsManager.saveFile();

    QCOMPARE(guard.stub.saveCallCount, 0);
    QCOMPARE(guard.stub.openCallCount, 0);
}

void TestCanvasInlineIC::testInlineICSaveMarksRootDirty()
{
    // When an inline IC tab saves, onChildICBlobSaved pushes a CanvasUpdateBlobCommand to the
    // root canvas's undo stack, which must mark the root tab as dirty (not clean).
    QuickWorkspaceManager wsManager;
    wsManager.newTab();
    auto *rootTab = wsManager.currentTab();
    auto *rootCanvas = rootTab->canvas();
    auto *ic = placeInlineTestIC(*rootCanvas, m_fixtureDir, "dirty_test");
    const QByteArray blob = rootCanvas->icRegistry()->blob("dirty_test");

    const QString rootPath = m_fixtureDir + "/dirty_root.panda";
    QFile::remove(rootPath);
    wsManager.save(rootPath);
    QVERIFY(rootCanvas->undoStack()->isClean());

    wsManager.openICInTab("dirty_test", ic->id(), blob);
    QVERIFY(wsManager.currentTab()->isInlineIC());

    wsManager.currentTab()->canvas()->addItem(ElementFactory::buildElement(ElementType::Led));

    ScopedFileDialogStub guard;
    wsManager.saveFile();
    QCOMPARE(guard.stub.saveCallCount, 0);

    QVERIFY2(!rootCanvas->undoStack()->isClean(),
             "Root undo stack should be dirty after inline IC child save");

    QFile::remove(rootPath);
}
