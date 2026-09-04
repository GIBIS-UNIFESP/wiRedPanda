// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasInlineIC.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QMouseEvent>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
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

/// Synthesizes a real double-click at \a worldPos and delivers it via
/// QCoreApplication::sendEvent(), the same technique TestCanvasItemInteraction.cpp uses --
/// QQuickItem::event() dispatches QEvent::MouseButtonDblClick to the exact protected
/// mouseDoubleClickEvent() a real window's input pipeline would call.
void sendDoubleClick(CanvasItem &canvas, const QPointF &worldPos)
{
    const QPointF screenPos = canvas.worldToScreen(worldPos);
    QMouseEvent event(QEvent::MouseButtonDblClick, screenPos, screenPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &event);
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
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *ic = placeInlineTestIC(*canvas, m_fixtureDir, "dedup_test");
    const QByteArray blob = canvas->icRegistry()->blob("dedup_test");

    const int initialTabs = controller.tabCount();

    controller.openICInTab("dedup_test", ic->id(), blob);
    QCOMPARE(controller.tabCount(), initialTabs + 1);

    // Reopening the same blob switches to the existing inline tab instead of duplicating it.
    controller.setCurrentIndex(0);
    controller.openICInTab("dedup_test", ic->id(), blob);
    QCOMPARE(controller.tabCount(), initialTabs + 1);
}

void TestCanvasInlineIC::testInlineTabTitleUpdatesAfterBlobRename()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *ic = placeInlineTestIC(*canvas, m_fixtureDir, "rename_tab_test");
    const QByteArray blob = canvas->icRegistry()->blob("rename_tab_test");

    controller.openICInTab("rename_tab_test", ic->id(), blob);
    auto *inlineTab = controller.currentTab();
    QCOMPARE(controller.tabTitle(inlineTab), QString("[rename_tab_test]"));

    // Rename the blob from the parent canvas (the inline tab stays open throughout) --
    // CanvasICRegistry::blobRenamed, connected in createNewTab(), must retitle it live.
    canvas->icRegistry()->renameBlob("rename_tab_test", "renamed_tab_test");

    QCOMPARE(controller.tabTitle(inlineTab), QString("[renamed_tab_test]"));
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
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *ic = placeInlineTestIC(*canvas, m_fixtureDir, "no_dialog_test");
    const QByteArray blob = canvas->icRegistry()->blob("no_dialog_test");

    controller.openICInTab("no_dialog_test", ic->id(), blob);
    QVERIFY(controller.currentTab()->isInlineIC());

    controller.currentTab()->canvas()->addItem(ElementFactory::buildElement(ElementType::And));

    ScopedFileDialogStub guard;
    controller.saveFile();

    QCOMPARE(guard.stub.saveCallCount, 0);
    QCOMPARE(guard.stub.openCallCount, 0);
}

void TestCanvasInlineIC::testInlineICSaveMarksRootDirty()
{
    // When an inline IC tab saves, onChildICBlobSaved pushes a CanvasUpdateBlobCommand to the
    // root canvas's undo stack, which must mark the root tab as dirty (not clean).
    QuickAppController controller;
    controller.newTab();
    auto *rootTab = controller.currentTab();
    auto *rootCanvas = rootTab->canvas();
    auto *ic = placeInlineTestIC(*rootCanvas, m_fixtureDir, "dirty_test");
    const QByteArray blob = rootCanvas->icRegistry()->blob("dirty_test");

    const QString rootPath = m_fixtureDir + "/dirty_root.panda";
    QFile::remove(rootPath);
    controller.saveCurrentTabAs(rootPath);
    QVERIFY(rootCanvas->undoStack()->isClean());

    controller.openICInTab("dirty_test", ic->id(), blob);
    QVERIFY(controller.currentTab()->isInlineIC());

    controller.currentTab()->canvas()->addItem(ElementFactory::buildElement(ElementType::Led));

    ScopedFileDialogStub guard;
    controller.saveFile();
    QCOMPARE(guard.stub.saveCallCount, 0);

    QVERIFY2(!rootCanvas->undoStack()->isClean(),
             "Root undo stack should be dirty after inline IC child save");

    QFile::remove(rootPath);
}

void TestCanvasInlineIC::testInlineICDropSaveCloseReopen()
{
    // Full scenario TestMainWindowGui::testInlineICDropSaveCloseReopen() guards: place an
    // embedded IC, open its inline tab, drop a file-backed IC into it, save (must auto-embed
    // the dropped IC into a self-contained blob -- QuickWorkSpace::save()'s inline branch),
    // close the tab, then reopen the same embedded IC and confirm the dropped IC survived as a
    // real, embedded element. See this class's header doc comment for why the ORIGINAL bug
    // mechanism (stale QTabWidget connections) can't recur under QuickWorkspaceManager.
    QuickAppController controller;
    controller.newTab();
    auto *rootTab = controller.currentTab();
    auto *rootCanvas = rootTab->canvas();
    rootCanvas->setContextDir(m_fixtureDir);

    auto *embeddedIC = placeInlineTestIC(*rootCanvas, m_fixtureDir, "parent_ic");
    const int embeddedICId = embeddedIC->id();
    const QByteArray blob = rootCanvas->icRegistry()->blob("parent_ic");
    QVERIFY(!blob.isEmpty());

    // A second, real fixture file to act as the file-based IC dropped into the inline tab.
    const QString dropFile = m_fixtureDir + "/drop_target.panda";
    {
        QuickWorkSpace fixtureWs;
        fixtureWs.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
        fixtureWs.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
        QVERIFY2(fixtureWs.save(dropFile) == QuickWorkSpace::SaveOutcome::Saved, "drop fixture write");
    }

    controller.openICInTab("parent_ic", embeddedICId, blob);
    QCOMPARE(controller.tabCount(), 2);
    auto *inlineTab = controller.currentTab();
    QVERIFY(inlineTab->isInlineIC());
    auto *inlineCanvas = inlineTab->canvas();

    auto *droppedIC = new IC();
    droppedIC->loadFile(dropFile, m_fixtureDir);
    droppedIC->setPos(200, 200);
    inlineCanvas->receiveCommand(new CanvasAddItemsCommand({droppedIC}, inlineCanvas));
    QVERIFY2(!droppedIC->isEmbedded(), "Dropped IC should be file-backed before save");

    ScopedFileDialogStub guard;
    controller.saveFile();
    QCOMPARE(guard.stub.saveCallCount, 0);
    QCOMPARE(guard.stub.openCallCount, 0);

    QVERIFY2(droppedIC->isEmbedded(), "IC should be embedded after inline save");
    QCOMPARE(droppedIC->blobName(), QString("drop_target"));
    QCOMPARE(controller.tabTitle(inlineTab), QString("[parent_ic]"));
    QVERIFY2(!rootCanvas->undoStack()->isClean(), "Root tab should be dirty after inline save");

    const QByteArray updatedBlob = rootCanvas->icRegistry()->blob("parent_ic");
    QVERIFY(!updatedBlob.isEmpty());

    const int inlineTabIndex = controller.indexOf(inlineTab);
    QVERIFY(controller.closeTab(inlineTabIndex)); // just saved, clean -- no confirm dialog needed
    QCOMPARE(controller.tabCount(), 1);
    QCOMPARE(controller.currentTab(), rootTab);

    bool reopenOK = false;
    try {
        controller.openICInTab("parent_ic", embeddedICId, updatedBlob);
        reopenOK = true;
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Reopen threw: %1").arg(e.what())));
    }
    QVERIFY(reopenOK);
    QCOMPARE(controller.tabCount(), 2);

    auto *reopenedTab = controller.currentTab();
    QVERIFY(reopenedTab->isInlineIC());

    bool foundDropped = false;
    for (auto *elm : reopenedTab->canvas()->elements()) {
        if (elm->isEmbedded() && elm->blobName() == "drop_target") {
            foundDropped = true;
        }
    }
    QVERIFY2(foundDropped, "Reopened inline tab should contain the embedded drop_target IC");

    QFile::remove(dropFile);
}

void TestCanvasInlineIC::testDoubleClickEmbeddedICOpensInlineTab()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    canvas->setSize(QSizeF(800, 600));

    auto *ic = placeInlineTestIC(*canvas, m_fixtureDir, "dblclick_embedded");
    QCOMPARE(controller.tabCount(), 1);

    sendDoubleClick(*canvas, ic->pos() + QPointF(10, 10));

    QCOMPARE(controller.tabCount(), 2);
    auto *inlineTab = controller.currentTab();
    QVERIFY(inlineTab->isInlineIC());
    QCOMPARE(inlineTab->inlineBlobName(), QString("dblclick_embedded"));
}

void TestCanvasInlineIC::testDoubleClickFileBackedICOpensNewTab()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    canvas->setSize(QSizeF(800, 600));

    auto *ic = new IC();
    ic->loadFile("test_circuit.panda", m_fixtureDir);
    ic->setPos(100, 100);
    canvas->receiveCommand(new CanvasAddItemsCommand({ic}, canvas));
    QVERIFY(!ic->isEmbedded());
    QCOMPARE(controller.tabCount(), 1);

    sendDoubleClick(*canvas, ic->pos() + QPointF(10, 10));

    QCOMPARE(controller.tabCount(), 2);
    auto *newTab = controller.currentTab();
    QVERIFY(!newTab->isInlineIC());
    QCOMPARE(newTab->fileInfo(), QFileInfo(m_fixtureDir + "/test_circuit.panda"));
}
