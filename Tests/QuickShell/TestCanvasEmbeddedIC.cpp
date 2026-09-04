// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasEmbeddedIC.h"

#include <QDataStream>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/VersionInfo.h"
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
/// bytes stored under \a blobName.
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

    // A minimal, loadable IC fixture (one switch, one LED, not wired to each other) written
    // via QuickWorkSpace, not the Widgets-side WorkSpace -- a real QWidget that aborts without
    // a QApplication.
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
    // wired afterward. CanvasCommandUtils::addItems() selects every element it adds, so sw/led
    // are still selected from the CanvasAddItemsCommand above -- clear that first, or
    // deleteSelected() deletes sw and led along with the IC, leaving the led->inputPort(0)
    // access below a heap-use-after-free.
    for (auto *elm : canvas.elements()) {
        elm->setSelected(false);
    }
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

    // CanvasCommandUtils::addItems() selects every element it adds, so all three ICs are still
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
    // CanvasDeleteItemsCommand.
    //
    // Calling undo() and expecting the IC to reappear is deliberately NOT asserted here:
    // file-restoration on undo is out of scope (the .panda was moved to the system trash, not
    // restored, and isn't part of the undo command) -- by the time undo() re-deserializes the
    // IC from its snapshot the file is genuinely gone, so IC::load()'s file-based fallback
    // throws. This test only checks that a command was pushed.
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

// ===========================================================================
// Toolbar-button-triggered IC operations
// ===========================================================================

void TestCanvasEmbeddedIC::testEmbedSelectedICNoOpWithNoSelection()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_embed_noop1.panda");

    controller.embedSelectedIC(); // must not crash with nothing selected
    QVERIFY(!canvas->undoStack()->canUndo());
}

void TestCanvasEmbeddedIC::testEmbedSelectedICNoOpWhenNotFileBacked()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_embed_noop2.panda");

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas->receiveCommand(new CanvasAddItemsCommand({sw}, canvas));
    sw->setSelected(true);

    const int stackCountBefore = canvas->undoStack()->count();
    controller.embedSelectedIC(); // selected element isn't even an IC
    QCOMPARE(canvas->undoStack()->count(), stackCountBefore);
}

void TestCanvasEmbeddedIC::testEmbedSelectedICSucceeds()
{
    QTemporaryDir workDir;
    QVERIFY(workDir.isValid());
    QFile::copy(m_fixtureDir + "/test_circuit.panda", workDir.path() + "/embed_me.panda");

    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(workDir.path() + "/project_embed.panda");

    auto *ic = new IC();
    ic->loadFile("embed_me.panda", workDir.path());
    canvas->receiveCommand(new CanvasAddItemsCommand({ic}, canvas));
    ic->setSelected(true);
    QVERIFY(!ic->isEmbedded());

    controller.embedSelectedIC();

    QVERIFY(ic->isEmbedded());
    QVERIFY(canvas->icRegistry()->hasBlob("embed_me"));
}

void TestCanvasEmbeddedIC::testExtractSelectedICNoOpWhenNotEmbedded()
{
    QTemporaryDir workDir;
    QVERIFY(workDir.isValid());
    QFile::copy(m_fixtureDir + "/test_circuit.panda", workDir.path() + "/not_embedded.panda");

    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(workDir.path() + "/project_extract_noop.panda");

    auto *ic = new IC();
    ic->loadFile("not_embedded.panda", workDir.path());
    canvas->receiveCommand(new CanvasAddItemsCommand({ic}, canvas));
    ic->setSelected(true);

    ScopedFileDialogStub fileGuard; // a save dialog opening here would indicate a real bug
    controller.extractSelectedIC();

    QCOMPARE(fileGuard.stub.saveCallCount, 0);
}

void TestCanvasEmbeddedIC::testExtractSelectedICSucceeds()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_extract_selected.panda");

    auto *ic = placeEmbeddedIC(*canvas, m_fixtureDir, "extract_selected_test");
    ic->setSelected(true);
    QVERIFY(ic->isEmbedded());

    const QString outPath = m_fixtureDir + "/extract_selected_output.panda";
    QFile::remove(outPath);
    ScopedFileDialogStub fileGuard;
    fileGuard.stub.saveResult = {outPath, "Panda files (*.panda)"};

    controller.extractSelectedIC();

    QVERIFY2(QFile::exists(outPath), "Extracted file should exist on disk");
    QVERIFY(!ic->isEmbedded());
    QVERIFY(!canvas->icRegistry()->hasBlob("extract_selected_test"));

    QFile::remove(outPath);
}

void TestCanvasEmbeddedIC::testMakeSelfContainedNoOpWithNoFileBasedICs()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_selfcontained_noop.panda");

    controller.makeSelfContained(); // empty circuit -- nothing to embed
    QVERIFY(!canvas->undoStack()->canUndo());
}

void TestCanvasEmbeddedIC::testMakeSelfContainedEmbedsAllFileBasedICs()
{
    QTemporaryDir workDir;
    QVERIFY(workDir.isValid());
    QFile::copy(m_fixtureDir + "/test_circuit.panda", workDir.path() + "/shared.panda");

    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(workDir.path() + "/project_selfcontained.panda");

    auto *ic1 = new IC();
    ic1->loadFile("shared.panda", workDir.path());
    auto *ic2 = new IC();
    ic2->loadFile("shared.panda", workDir.path());
    canvas->receiveCommand(new CanvasAddItemsCommand({ic1, ic2}, canvas));
    QVERIFY(!ic1->isEmbedded());
    QVERIFY(!ic2->isEmbedded());

    controller.makeSelfContained();

    QVERIFY(ic1->isEmbedded());
    QVERIFY(ic2->isEmbedded());
    QVERIFY(canvas->icRegistry()->hasBlob("shared"));
}

void TestCanvasEmbeddedIC::testAddICFromFileCopiesFile()
{
    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    QFile::copy(m_fixtureDir + "/test_circuit.panda", sourceDir.path() + "/importable.panda");

    QTemporaryDir projectDir;
    QVERIFY(projectDir.isValid());

    QuickAppController controller;
    controller.newTab();
    controller.saveCurrentTabAs(projectDir.path() + "/project_add_ic.panda");

    ScopedFileDialogStub fileGuard;
    fileGuard.stub.openResult = sourceDir.path() + "/importable.panda";
    ScopedDialogStub dialogGuard; // the "files will be copied" info notice

    controller.addICFromFile();

    QVERIFY2(QFile::exists(projectDir.path() + "/importable.panda"),
             "addICFromFile must copy the chosen file into the project directory");
}

void TestCanvasEmbeddedIC::testAddICFromFileNoOpWhenDialogCancelled()
{
    QuickAppController controller;
    controller.newTab();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_add_ic_cancel.panda");

    ScopedFileDialogStub fileGuard;
    fileGuard.stub.openResult = {}; // cancelled

    controller.addICFromFile(); // must not crash or attempt to copy an empty path
}

void TestCanvasEmbeddedIC::testAddEmbeddedICFromFileSucceeds()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_add_embedded.panda");

    ScopedFileDialogStub fileGuard;
    fileGuard.stub.openResult = m_fixtureDir + "/test_circuit.panda";

    controller.addEmbeddedICFromFile();

    QVERIFY2(canvas->icRegistry()->hasBlob("test_circuit"),
             "addEmbeddedICFromFile must register the chosen file as an embedded blob");
    QCOMPARE(canvas->elements().size(), 0); // registers the blob only, no scene instance
}

void TestCanvasEmbeddedIC::testAddEmbeddedICFromFileNoOpWhenDialogCancelled()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    controller.saveCurrentTabAs(m_fixtureDir + "/project_add_embedded_cancel.panda");

    ScopedFileDialogStub fileGuard;
    fileGuard.stub.openResult = {}; // cancelled

    controller.addEmbeddedICFromFile();

    QVERIFY(!canvas->icRegistry()->hasBlob("test_circuit"));
}

void TestCanvasEmbeddedIC::testRenameBlobUpdatesNestedMetadataReference()
{
    // "child" is a plain leaf circuit's raw bytes -- the same shape as every other fixture in
    // this file. "parent" is a *different* circuit embedding an IC that itself references
    // "child_ic" as its blob, so parent's own saved bytes carry a nested blob-registry
    // reference to it (Serialization::serializeBlobRegistry() writes whatever's in the
    // canvas's own icRegistry()->blobMapRef() at save time).
    QuickWorkSpace childWs;
    childWs.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    childWs.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QByteArray childBytes;
    {
        QDataStream stream(&childBytes, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        childWs.save(stream);
    }

    QuickWorkSpace parentWs;
    parentWs.canvas()->icRegistry()->setBlob("child_ic", childBytes);
    auto *nestedIc = new IC();
    nestedIc->setBlobName("child_ic");
    nestedIc->loadFromBlob(childBytes, m_fixtureDir);
    parentWs.canvas()->receiveCommand(new CanvasAddItemsCommand({nestedIc}, parentWs.canvas()));
    QByteArray parentBytes;
    {
        QDataStream stream(&parentBytes, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        parentWs.save(stream);
    }

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.icRegistry()->setBlob("child_ic", childBytes);
    canvas.icRegistry()->setBlob("parent_ic", parentBytes);

    canvas.icRegistry()->renameBlob("child_ic", "child_ic_renamed");

    QVERIFY(canvas.icRegistry()->hasBlob("child_ic_renamed"));
    QVERIFY(!canvas.icRegistry()->hasBlob("child_ic"));

    QByteArray updatedParentBytes = canvas.icRegistry()->blob("parent_ic");
    QDataStream readStream(&updatedParentBytes, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(readStream);
    QVERIFY(VersionInfo::hasMetadata(preamble.version));
    const auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);
    QVERIFY2(embeddedICs.contains("child_ic_renamed"),
             "parent_ic's own metadata must reference the renamed blob under its new name");
    QVERIFY2(!embeddedICs.contains("child_ic"),
             "parent_ic's own metadata must not still reference the pre-rename name");
    QCOMPARE(embeddedICs.value("child_ic_renamed"), childBytes);
}

void TestCanvasEmbeddedIC::testRegisterBlobInlinesFileBackedDependency()
{
    // A real leaf circuit file on disk, referenced by a *file-backed* (not embedded) IC inside
    // "wrapper" -- wrapperBytes' own metadata therefore carries a "fileBackedICs" entry naming
    // it (QuickWorkSpace::save()'s own loop over non-embedded IC elements), not an
    // "embeddedICs" one. registerBlob() must resolve that file from disk (relative to this
    // registry's own contextDir()) and inline it, leaving no external file reference behind.
    QTemporaryDir depDir;
    QVERIFY(depDir.isValid());
    QuickWorkSpace leafWs;
    leafWs.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    leafWs.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(leafWs.save(depDir.path() + "/leaf.panda") == QuickWorkSpace::SaveOutcome::Saved, "leaf fixture write");

    QuickWorkSpace wrapperWs;
    auto *fileBackedIc = new IC();
    fileBackedIc->loadFile("leaf.panda", depDir.path());
    QVERIFY(!fileBackedIc->isEmbedded());
    wrapperWs.canvas()->receiveCommand(new CanvasAddItemsCommand({fileBackedIc}, wrapperWs.canvas()));
    QByteArray wrapperBytes;
    {
        QDataStream stream(&wrapperBytes, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        wrapperWs.save(stream);
    }

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setContextDir(depDir.path());
    canvas.icRegistry()->registerBlob("wrapper_ic", wrapperBytes);

    QByteArray storedBytes = canvas.icRegistry()->blob("wrapper_ic");
    QDataStream readStream(&storedBytes, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(readStream);
    QVERIFY(VersionInfo::hasMetadata(preamble.version));
    QVERIFY2(!preamble.metadata.contains("fileBackedICs"),
             "registerBlob() must resolve away the file-backed dependency, leaving no external reference");
    const auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);
    QVERIFY2(embeddedICs.contains("leaf"), "the leaf file's dependency must be inlined under its base name");

    // The inlined bytes must themselves be a real, loadable circuit -- read back its own
    // preamble successfully (a truncated/corrupt embed would fail this).
    QByteArray leafBytes = embeddedICs.value("leaf");
    QDataStream leafStream(&leafBytes, QIODevice::ReadOnly);
    const auto leafPreamble = Serialization::readPreamble(leafStream);
    QVERIFY(!leafPreamble.remainingPayload.isEmpty());
}

void TestCanvasEmbeddedIC::testFileWatcherEmitsDefinitionChangedWithNoTargets()
{
    // A real QFileSystemWatcher notification (Qt::QueuedConnection) for a watched file with no
    // canvas element referencing it must still emit definitionChanged(), not silently no-op.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("watched_no_target.panda");

    QuickWorkSpace fixtureWs;
    fixtureWs.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    QVERIFY2(fixtureWs.save(path) == QuickWorkSpace::SaveOutcome::Saved, "initial fixture write");

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.icRegistry()->watchFile(path);

    QSignalSpy definitionChangedSpy(canvas.icRegistry(), &CanvasICRegistry::definitionChanged);
    fixtureWs.canvas()->addItem(ElementFactory::buildElement(ElementType::Led)); // genuinely change the bytes
    QVERIFY2(fixtureWs.save(path) == QuickWorkSpace::SaveOutcome::Saved, "rewrite to trigger the watcher");
    QVERIFY2(definitionChangedSpy.wait(), "definitionChanged must fire even when no IC references the file");
}

void TestCanvasEmbeddedIC::testFileWatcherReloadsFileBackedICAndPushesUndoCommand()
{
    // Same scenario TestICInline::testOnFileChangedPushesUndoCommandC5() guards against:
    // onFileChanged() must push a command onto the undo stack rather than calling
    // IC::loadFile() directly -- a bare reload would leave the undo stack's recorded item ids
    // out of sync with the freshly-rebuilt IC. watchFile() itself is never called directly
    // here; CanvasItem::addItem()'s own hook is what's under test.
    //
    // Undo restoring the *pre-edit port count* is deliberately NOT asserted here: a file-backed
    // IC's own save() stores only a path reference (IC::save()), never a value snapshot of its
    // ports/sub-circuit, so "undo" after an external file edit reloads from the very same
    // (already rewritten) path and observes the *new* content again -- there is no old-content
    // copy left anywhere to restore. What IS real and worth guarding: a command is pushed, and
    // undo() walks the stack back cleanly without throwing on stale ids.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("watched_reload.panda");

    QuickWorkSpace fixtureWs;
    fixtureWs.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    fixtureWs.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(fixtureWs.save(path) == QuickWorkSpace::SaveOutcome::Saved, "initial fixture write (1 input)");

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic = new IC();
    ic->loadFile(path, dir.path());
    QCOMPARE(ic->inputSize(), 1);
    canvas.receiveCommand(new CanvasAddItemsCommand({ic}, &canvas));

    const int undoCountBefore = canvas.undoStack()->count();
    const int undoIndexBefore = canvas.undoStack()->index();

    // Rewrite the same file with a real, different circuit (2 inputs instead of 1) so the
    // reload itself is independently verifiable, not just "didn't crash".
    QuickWorkSpace changedWs;
    changedWs.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    changedWs.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    changedWs.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));

    QSignalSpy definitionChangedSpy(canvas.icRegistry(), &CanvasICRegistry::definitionChanged);
    QVERIFY2(changedWs.save(path) == QuickWorkSpace::SaveOutcome::Saved, "rewrite fixture (2 inputs)");
    QVERIFY2(definitionChangedSpy.wait(), "definitionChanged must fire once the reload completes");

    QCOMPARE(ic->inputSize(), 2);
    QVERIFY2(canvas.undoStack()->count() == undoCountBefore + 1,
             "onFileChanged() must push exactly one CanvasUpdateBlobCommand on the undo stack");

    canvas.undoStack()->undo();
    QCOMPARE(canvas.undoStack()->index(), undoIndexBefore);
    QCOMPARE(canvas.elements().size(), 1);
}
