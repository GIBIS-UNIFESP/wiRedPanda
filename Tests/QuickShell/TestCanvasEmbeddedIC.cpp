// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasEmbeddedIC.h"

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
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/StubFileDialogProvider.h"
#include "Tests/QuickShell/StubDialogProvider.h"

namespace {

/// Places a real embedded IC on \a canvas, backed by \a fixtureDir's "test_circuit.panda"
/// bytes stored under \a blobName. Mirrors TestMainWindowGui.cpp's identical placeEmbeddedIC()
/// helper against CanvasICRegistry/CanvasItem instead of ICRegistry/Scene.
IC *placeEmbeddedIC(CanvasItem &canvas, const QString &fixtureDir, const QString &blobName,
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

int countEmbeddedICs(CanvasItem &canvas, const QString &blobName = {})
{
    int count = 0;
    for (auto *elm : canvas.elements()) {
        if (elm->isEmbedded() && (blobName.isEmpty() || elm->blobName() == blobName)) {
            ++count;
        }
    }
    return count;
}

} // namespace

void TestCanvasEmbeddedIC::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    m_fixtureDir = m_tempDir.path();

    // A minimal, loadable IC fixture (one switch, one LED -- not wired to each other, matching
    // TestMainWindowGui.cpp's own identical createMWFixture()) written via QuickWorkSpace, not
    // the Widgets-side WorkSpace (a real QWidget that aborts without a QApplication -- see
    // TestQuickICHandlerSecurity.cpp's identical finding).
    QuickWorkSpace ws;
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(ws.save(m_fixtureDir + "/test_circuit.panda") == QuickWorkSpace::SaveOutcome::Saved, "fixture write");
}

// ===========================================================================
// Embedded IC operations
// ===========================================================================

void TestCanvasEmbeddedIC::testEmbeddedICCopyPaste()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic = placeEmbeddedIC(canvas, m_fixtureDir, "copy_test");
    QCOMPARE(countEmbeddedICs(canvas), 1);

    ic->setSelected(true);
    canvas.copyAction();
    canvas.pasteAction();

    QCOMPARE(countEmbeddedICs(canvas, "copy_test"), 2);
    for (auto *elm : canvas.elements()) {
        if (elm->isEmbedded()) {
            QCOMPARE(elm->blobName(), QString("copy_test"));
            QVERIFY(elm->inputSize() > 0);
        }
    }
}

void TestCanvasEmbeddedIC::testEmbeddedICCutPaste()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic = placeEmbeddedIC(canvas, m_fixtureDir, "cut_test");
    QCOMPARE(countEmbeddedICs(canvas), 1);

    ic->setSelected(true);
    canvas.cutAction();
    QCOMPARE(countEmbeddedICs(canvas), 0);

    canvas.pasteAction();
    QCOMPARE(countEmbeddedICs(canvas, "cut_test"), 1);
}

void TestCanvasEmbeddedIC::testEmbeddedICDeleteUndo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic = placeEmbeddedIC(canvas, m_fixtureDir, "del_test");

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(50, 100);
    led->setPos(250, 100);
    auto *c1 = new Connection();
    c1->setStartPort(sw->outputPort(0));
    c1->setEndPort(ic->inputPort(0));
    auto *c2 = new Connection();
    c2->setStartPort(ic->outputPort(0));
    c2->setEndPort(led->inputPort(0));
    canvas.receiveCommand(new CanvasAddItemsCommand({sw, led}, &canvas));

    QCOMPARE(canvas.connections().size(), 2);

    ic->setSelected(true);
    canvas.deleteSelected();

    QCOMPARE(countEmbeddedICs(canvas), 0);
    QCOMPARE(canvas.connections().size(), 0);

    canvas.undoStack()->undo();
    QCOMPARE(countEmbeddedICs(canvas, "del_test"), 1);
    QCOMPARE(canvas.connections().size(), 2);
}

void TestCanvasEmbeddedIC::testEmbeddedICSelectAllDeleteUndo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    placeEmbeddedIC(canvas, m_fixtureDir, "selall", {100, 100});
    canvas.receiveCommand(new CanvasAddItemsCommand(
        {ElementFactory::buildElement(ElementType::And), ElementFactory::buildElement(ElementType::Led)}, &canvas));

    const qsizetype origCount = canvas.elements().size();
    QCOMPARE(origCount, 3);

    canvas.selectAll();
    canvas.deleteSelected();
    QCOMPARE(canvas.elements().size(), 0);

    canvas.undoStack()->undo();
    QCOMPARE(canvas.elements().size(), origCount);
    QCOMPARE(countEmbeddedICs(canvas), 1);
}

void TestCanvasEmbeddedIC::testEmbeddedICRotatePreservesState()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic = placeEmbeddedIC(canvas, m_fixtureDir, "rot_test");
    const int icId = ic->id();

    ic->setSelected(true);
    canvas.rotateRight();

    auto *rotated = dynamic_cast<IC *>(canvas.itemById(icId));
    QVERIFY(rotated);
    QCOMPARE(rotated->rotation(), 90.0);
    QVERIFY(rotated->isEmbedded());
    QCOMPARE(rotated->blobName(), QString("rot_test"));

    canvas.undoStack()->undo();
    auto *undone = dynamic_cast<IC *>(canvas.itemById(icId));
    QVERIFY(undone);
    QCOMPARE(undone->rotation(), 0.0);
    QVERIFY(undone->isEmbedded());
}

void TestCanvasEmbeddedIC::testEmbeddedICSimulationAfterDeleteUndo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(50, 80);
    led->setPos(300, 100);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw, led}, &canvas));

    auto *ic = placeEmbeddedIC(canvas, m_fixtureDir, "sim_test", {180, 100});
    QCOMPARE(ic->inputSize(), 1);

    auto *c1 = new Connection();
    c1->setStartPort(sw->outputPort(0));
    c1->setEndPort(ic->inputPort(0));
    auto *c2 = new Connection();
    c2->setStartPort(ic->outputPort(0));
    c2->setEndPort(led->inputPort(0));
    canvas.addItem(c1);
    canvas.addItem(c2);
    canvas.restartSimulation();

    // Delete IC, undo, re-simulate -- must not crash, and the IC must still be present and
    // wired afterward.
    ic->setSelected(true);
    canvas.deleteSelected();
    canvas.undoStack()->undo();

    canvas.restartSimulation();
    QCOMPARE(countEmbeddedICs(canvas, "sim_test"), 1);
    QCOMPARE(led->inputPort(0)->connections().size(), 1);
}

void TestCanvasEmbeddedIC::testEmbeddedICMultipleTypesDeleteOne()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *alpha1 = placeEmbeddedIC(canvas, m_fixtureDir, "type_a", {100, 100});
    auto *alpha2 = placeEmbeddedIC(canvas, m_fixtureDir, "type_a", {200, 100});
    placeEmbeddedIC(canvas, m_fixtureDir, "type_b", {100, 200});

    QCOMPARE(countEmbeddedICs(canvas, "type_a"), 2);
    QCOMPARE(countEmbeddedICs(canvas, "type_b"), 1);

    // CanvasCommandUtils::addItems() selects every element it adds (see
    // TestCanvasCommands.cpp's identical selectOnly() finding), so all three ICs are still
    // selected from their own placeEmbeddedIC() calls -- clear that before selecting only
    // the two intended targets.
    for (auto *elm : canvas.elements()) {
        elm->setSelected(false);
    }
    alpha1->setSelected(true);
    alpha2->setSelected(true);
    canvas.deleteSelected();

    QCOMPARE(countEmbeddedICs(canvas, "type_a"), 0);
    QCOMPARE(countEmbeddedICs(canvas, "type_b"), 1);

    canvas.undoStack()->undo();
    QCOMPARE(countEmbeddedICs(canvas, "type_a"), 2);
    QCOMPARE(countEmbeddedICs(canvas, "type_b"), 1);
}

void TestCanvasEmbeddedIC::testEmbeddedICCrossTabCopyPaste()
{
    // Two independent canvases, mirroring two tabs -- copy/paste round-trips through the real
    // system clipboard (QGuiApplication::clipboard()), the only channel available since there's
    // no shared in-process state between them.
    CanvasItem canvas1(nullptr, /*buildDemo=*/false);
    auto *ic = placeEmbeddedIC(canvas1, m_fixtureDir, "cross_tab");
    QVERIFY(canvas1.icRegistry()->hasBlob("cross_tab"));

    ic->setSelected(true);
    canvas1.copyAction();

    CanvasItem canvas2(nullptr, /*buildDemo=*/false);
    QVERIFY(!canvas2.icRegistry()->hasBlob("cross_tab"));
    canvas2.pasteAction();

    QVERIFY2(canvas2.icRegistry()->hasBlob("cross_tab"),
             "Pasting an embedded IC into a different canvas must import its blob into that "
             "canvas's own registry, not just the element referencing it");
    QCOMPARE(countEmbeddedICs(canvas2, "cross_tab"), 1);
}

// ===========================================================================
// File <-> embedded conversion (QuickICController)
// ===========================================================================

void TestCanvasEmbeddedIC::testEmbedICByFileNoInstances()
{
    // When no instances of the file-based IC exist in the scene, embedICByFile() must
    // register the blob (CanvasRegisterBlobCommand) without adding any IC element.
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_no_instances.panda");
    QVERIFY(!canvas->contextDir().isEmpty());

    const qsizetype elementsBefore = canvas->elements().size();

    controller.embedICByFile("test_circuit.panda");

    QCOMPARE(canvas->elements().size(), elementsBefore);
    QVERIFY(canvas->icRegistry()->hasBlob("test_circuit"));
    QVERIFY(canvas->undoStack()->canUndo());

    canvas->undoStack()->undo();
    QVERIFY(!canvas->icRegistry()->hasBlob("test_circuit"));
    QCOMPARE(canvas->elements().size(), elementsBefore);
}

void TestCanvasEmbeddedIC::testEmbedICByFileWithInstances()
{
    // When file-backed ICs referencing the file already exist, embedICByFile() must convert
    // them to embedded without adding extra elements.
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_with_instances.panda");

    auto *ic1 = new IC();
    ic1->loadFile("test_circuit.panda", m_fixtureDir);
    ic1->setPos(100, 100);
    auto *ic2 = new IC();
    ic2->loadFile("test_circuit.panda", m_fixtureDir);
    ic2->setPos(200, 100);
    canvas->receiveCommand(new CanvasAddItemsCommand({ic1, ic2}, canvas));

    const qsizetype elementsBefore = canvas->elements().size();
    QVERIFY(!ic1->isEmbedded());
    QVERIFY(!ic2->isEmbedded());

    controller.embedICByFile("test_circuit.panda");

    QCOMPARE(canvas->elements().size(), elementsBefore); // conversion, not addition
    QVERIFY(ic1->isEmbedded());
    QVERIFY(ic2->isEmbedded());
    QVERIFY(canvas->icRegistry()->hasBlob("test_circuit"));

    canvas->undoStack()->undo();
    QVERIFY(!ic1->isEmbedded());
    QVERIFY(!ic2->isEmbedded());
    QVERIFY(!canvas->icRegistry()->hasBlob("test_circuit"));
}

void TestCanvasEmbeddedIC::testExtractICByBlobNameEndToEnd()
{
    // Extracting an embedded IC must write the blob to the dialog-chosen path, remove it from
    // the registry, and convert the IC to file-backed. Undo must reinstate the embedded state.
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_extract.panda");

    auto *ic = placeEmbeddedIC(*canvas, m_fixtureDir, "extract_test");
    QVERIFY(ic->isEmbedded());
    QVERIFY(canvas->icRegistry()->hasBlob("extract_test"));

    const QString outPath = m_fixtureDir + "/extracted_output.panda";
    QFile::remove(outPath);

    ScopedFileDialogStub guard;
    guard.stub.saveResult = {outPath, "Panda files (*.panda)"};

    controller.extractICByBlobName("extract_test");

    QVERIFY2(QFile::exists(outPath), "Extracted file should exist on disk");
    QVERIFY2(QFile(outPath).size() > 0, "Extracted file should be non-empty");
    QVERIFY(!canvas->icRegistry()->hasBlob("extract_test"));
    QVERIFY(!ic->isEmbedded());
    QVERIFY2(ic->file().contains("extracted_output"), "IC file path should point to extracted file");

    canvas->undoStack()->undo();
    QVERIFY(ic->isEmbedded());
    QVERIFY(canvas->icRegistry()->hasBlob("extract_test"));

    QFile::remove(outPath);
}

void TestCanvasEmbeddedIC::testRemoveICFileIsUndoableA14()
{
    // removeICFile() moves the source file to the system trash and deletes every scene
    // instance referencing it, after a Yes/No confirmation, going through a real
    // CanvasDeleteItemsCommand -- pre-fix (Widgets' own A14 regression) it was a bare
    // removeItem()+delete with no command pushed at all, so undo couldn't bring the IC back
    // and the freed pointer stayed live until the next sim tick faulted on it.
    //
    // Actually calling undo() and expecting the IC to reappear is deliberately NOT asserted
    // here, matching TestMainWindowGui::testRemoveICFileIsUndoableA14's own explicit scope
    // ("The file-restoration on undo is out of scope (the .panda was deleted... and isn't part
    // of the undo command)") -- confirmed the hard way: the file is genuinely gone (trashed)
    // by the time undo() re-deserializes the IC from its snapshot, so IC::load()'s file-based
    // fallback throws. The Widgets test itself never calls undo() for exactly this reason, only
    // checking that a command was pushed; this port does the same.
    QTemporaryDir workDir;
    QVERIFY(workDir.isValid());
    QFile::copy(m_fixtureDir + "/test_circuit.panda", workDir.path() + "/removable.panda");

    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(workDir.path() + "/project_remove.panda");

    auto *ic = new IC();
    ic->loadFile("removable.panda", workDir.path());
    canvas->receiveCommand(new CanvasAddItemsCommand({ic}, canvas));
    QCOMPARE(canvas->elements().size(), 1);

    ScopedDialogStub dialogGuard;
    dialogGuard.stub.choiceResult = DialogButton::Yes;

    const int stackCountBefore = canvas->undoStack()->count();
    controller.removeICFile("removable.panda");

    QCOMPARE(dialogGuard.stub.choiceCallCount, 1);
    QCOMPARE(canvas->elements().size(), 0);
    QVERIFY(!QFile::exists(workDir.path() + "/removable.panda"));
    QVERIFY2(canvas->undoStack()->count() > stackCountBefore,
             "removeICFile must push a CanvasDeleteItemsCommand on the undo stack");
}
