// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickElementEditor.h"

#include <QFile>
#include <QSignalSpy>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickElementEditor.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/StubFileDialogProvider.h"
#include "Tests/QuickShell/StubDialogProvider.h"

namespace {

/// Deselects every element on \a canvas but \a target, then re-syncs \a editor -- setCanvas()
/// is idempotent and re-reads the current selection, so it's the sanctioned public way to make
/// the editor notice a selection change made directly via setSelected() (CanvasAddItemsCommand's
/// own auto-select doesn't emit CanvasItem::selectionChanged() itself -- only real user-gesture
/// call sites like addElementFromPalette() do, see CanvasItem.cpp).
void selectOnly(CanvasItem &canvas, QuickElementEditor &editor, GraphicElement *target)
{
    for (auto *elm : canvas.elements()) {
        elm->setSelected(elm == target);
    }
    editor.setCanvas(&canvas);
}

IC *placeEmbeddedTestIC(CanvasItem &canvas, const QString &fixtureDir, const QString &blobName,
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

void TestQuickElementEditor::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    m_fixtureDir = m_tempDir.path();

    QuickWorkSpace ws;
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(ws.save(m_fixtureDir + "/test_circuit.panda") == QuickWorkSpace::SaveOutcome::Saved, "fixture write");
}

void TestQuickElementEditor::testSetCanvasBindsSelectionAndRefreshesOnChange()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(!editor.hasSelection());

    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas)); // auto-selected

    editor.setCanvas(&canvas); // re-sync, see selectOnly()'s doc comment
    QVERIFY(editor.hasSelection());
    QVERIFY(!editor.elementTypeTitle().isEmpty());
}

void TestQuickElementEditor::testEmptySelectionClearsFields()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.hasSelection());

    canvas.clearSelection(); // emits selectionChanged() for real
    QVERIFY(!editor.hasSelection());
    QVERIFY(editor.label().isEmpty());
    QVERIFY(editor.elementTypeTitle().isEmpty());
}

void TestQuickElementEditor::testLabelSetAppliesUndoably()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isLabelVisible());

    editor.setLabel("TestLabel123");
    QCOMPARE(led->label(), QString("TestLabel123"));

    canvas.undoStack()->undo();
    QVERIFY(led->label() != QString("TestLabel123"));
}

void TestQuickElementEditor::testColorSetAppliesUndoablyAndColorOptionsNonEmpty()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isColorVisible());
    QVERIFY(QuickElementEditor::colorOptions().size() >= 2);

    const QString original = led->color();
    editor.setColor("Blue");
    QCOMPARE(led->color(), QString("Blue"));

    canvas.undoStack()->undo();
    QCOMPARE(led->color(), original);
}

void TestQuickElementEditor::testTriggerSetForcesUppercaseAndAppliesUndoably()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isTriggerVisible());

    editor.setTrigger("a"); // lowercase -- must be forced to "A"
    QCOMPARE(sw->trigger(), QKeySequence(Qt::Key_A));
    QCOMPARE(editor.trigger(), QString("A"));

    canvas.undoStack()->undo();
    QVERIFY(sw->trigger() != QKeySequence(Qt::Key_A));
}

void TestQuickElementEditor::testTriggerRejectsMultiCharacter()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);

    const int undoCountBefore = canvas.undoStack()->count();
    editor.setTrigger("AB"); // longer than one char -- setTrigger() must no-op

    QCOMPARE(canvas.undoStack()->count(), undoCountBefore);
    QVERIFY(editor.trigger().isEmpty());
}

void TestQuickElementEditor::testDelaySetAppliesUndoably()
{
    // Only Clock has ElementConstraints::hasDelay == true -- see this class's own header doc
    // comment for why this deliberately doesn't mirror TestMainWindowGui::testChangeDelay()'s
    // use of Node.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *clock = ElementFactory::buildElement(ElementType::Clock);
    canvas.receiveCommand(new CanvasAddItemsCommand({clock}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isDelayVisible());

    const double before = clock->delay();
    editor.setDelaySteps(editor.delaySteps() + 1);
    QVERIFY(clock->delay() != before);

    canvas.undoStack()->undo();
    QCOMPARE(clock->delay(), before);
}

void TestQuickElementEditor::testInputSizeChangeIsUndoable()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isInputSizeVisible());
    QVERIFY(!editor.inputSizeOptions().isEmpty());

    const int before = andGate->inputSize();
    editor.setInputSize(before + 1);
    QCOMPARE(andGate->inputSize(), before + 1);

    canvas.undoStack()->undo();
    QCOMPARE(andGate->inputSize(), before);
}

void TestQuickElementEditor::testOutputSizeChangeIsUndoable()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
    canvas.receiveCommand(new CanvasAddItemsCommand({demux}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isOutputSizeVisible());
    QVERIFY(!editor.outputSizeOptions().isEmpty());

    const int before = demux->outputSize();
    editor.setOutputSize(before + 1);
    QCOMPARE(demux->outputSize(), before + 1);

    canvas.undoStack()->undo();
    QCOMPARE(demux->outputSize(), before);
}

void TestQuickElementEditor::testLockedStateChangeIsUndoable()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    auto *input = qobject_cast<GraphicElementInput *>(sw);
    QVERIFY(input);

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isLockedVisible());
    QVERIFY(!input->isLocked());

    editor.setLockedState(2); // Qt::Checked
    QVERIFY(input->isLocked());

    canvas.undoStack()->undo();
    QVERIFY(!input->isLocked());
}

void TestQuickElementEditor::testOutputValueChangeIsUndoable()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    auto *input = qobject_cast<GraphicElementInput *>(sw);
    QVERIFY(input);

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isOutputValueVisible());
    QVERIFY(!input->isOn());

    editor.setOutputValue(1);
    QVERIFY(input->isOn());

    canvas.undoStack()->undo();
    QVERIFY(!input->isOn());
}

void TestQuickElementEditor::testWirelessModeChangeSeversStaleConnection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *node = ElementFactory::buildElement(ElementType::Node);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(200, 0);

    auto *conn = new Connection();
    conn->setStartPort(node->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    canvas.receiveCommand(new CanvasAddItemsCommand({node, led}, &canvas));
    QCOMPARE(canvas.connections().size(), 1);

    QuickElementEditor editor;
    selectOnly(canvas, editor, node);
    QVERIFY(editor.isWirelessModeVisible());

    // Tx replaces the node's own output port with wireless routing -- the wire from that port
    // to the Led is no longer meaningful and must be severed as part of the same undo step.
    editor.setWirelessMode(1); // WirelessMode::Tx
    QCOMPARE(static_cast<Node *>(node)->wirelessMode(), WirelessMode::Tx);
    QCOMPARE(canvas.connections().size(), 0);

    canvas.undoStack()->undo(); // one macro undoes both the mode change and the wire deletion
    QCOMPARE(static_cast<Node *>(node)->wirelessMode(), WirelessMode::None);
    QCOMPARE(canvas.connections().size(), 1);
}

void TestQuickElementEditor::testWirelessModeRejectsDuplicateTxLabel()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *node1 = ElementFactory::buildElement(ElementType::Node);
    auto *node2 = ElementFactory::buildElement(ElementType::Node);
    node2->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({node1}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({node2}, &canvas));

    QuickElementEditor editor;
    selectOnly(canvas, editor, node1);
    editor.setLabel("CH1");
    editor.setWirelessMode(1); // Tx -- the only Tx with this label so far, succeeds
    QCOMPARE(static_cast<Node *>(node1)->wirelessMode(), WirelessMode::Tx);

    selectOnly(canvas, editor, node2);
    editor.setLabel("CH1"); // same label as node1

    ScopedDialogStub guard;
    editor.setWirelessMode(1); // Tx again -- must be rejected, node1 already owns "CH1"

    QCOMPARE(guard.stub.choiceCallCount, 1);
    QCOMPARE(static_cast<Node *>(node2)->wirelessMode(), WirelessMode::None);
}

void TestQuickElementEditor::testPrepareContextMenuPopulatesGateMorphCandidates()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.canMorph());

    editor.prepareContextMenu(andGate);

    const auto candidates = editor.morphCandidates();
    QVERIFY(!candidates.isEmpty());
    bool foundOr = false;
    for (const auto &candidate : candidates) {
        if (static_cast<ElementType>(candidate.type()) == ElementType::Or) {
            foundOr = true;
        }
    }
    QVERIFY2(foundOr, "And's morph candidates should include Or (multi-input Gate group)");
}

void TestQuickElementEditor::testPrepareContextMenuExcludesSelectionsOwnType()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    editor.prepareContextMenu(andGate);

    for (const auto &candidate : editor.morphCandidates()) {
        QVERIFY2(static_cast<ElementType>(candidate.type()) != ElementType::And,
                 "morphCandidates() must not offer morphing to the selection's own current type");
    }
}

void TestQuickElementEditor::testMorphSelectionToAppliesToWholeSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));
    const int id = andGate->id();

    QuickElementEditor editor;
    editor.setCanvas(&canvas);

    editor.morphSelectionTo(static_cast<int>(ElementType::Or));

    auto *morphed = dynamic_cast<GraphicElement *>(canvas.itemById(id));
    QVERIFY(morphed);
    QCOMPARE(morphed->elementType(), ElementType::Or);

    canvas.undoStack()->undo();
    auto *reverted = dynamic_cast<GraphicElement *>(canvas.itemById(id));
    QVERIFY(reverted);
    QCOMPARE(reverted->elementType(), ElementType::And);
}

void TestQuickElementEditor::testChangeAppearancePicksFileAndAppliesUndoably()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isAppearanceVisible());

    ScopedFileDialogStub guard;
    // A real, loadable image -- setAppearance() actually decodes this as a pixmap (throws
    // Pandaception on a bogus path/content), unlike FileDialogProvider's own stub, which never
    // touches the filesystem.
    guard.stub.openResult = QStringLiteral(":/Components/Logic/and.svg");

    const int undoCountBefore = canvas.undoStack()->count();
    editor.changeAppearance();

    QCOMPARE(guard.stub.openCallCount, 1);
    QVERIFY(guard.stub.lastOpenCall.filter.contains("Images"));
    QCOMPARE(canvas.undoStack()->count(), undoCountBefore + 1);

    canvas.undoStack()->undo();
}

void TestQuickElementEditor::testChangeAppearanceCancelledDialogIsNoop()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);

    ScopedFileDialogStub guard;
    guard.stub.openResult = QString(); // cancelled

    const int undoCountBefore = canvas.undoStack()->count();
    editor.changeAppearance();

    QCOMPARE(guard.stub.openCallCount, 1);
    QCOMPARE(canvas.undoStack()->count(), undoCountBefore);
}

void TestQuickElementEditor::testResetAppearanceRestoresDefault()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);

    const int undoCountBefore = canvas.undoStack()->count();
    editor.resetAppearance();

    // resetAppearance() always pushes a CanvasUpdateCommand (mirrors defaultAppearance()'s
    // unconditional apply()) -- the element is still selected and valid afterward.
    QCOMPARE(canvas.undoStack()->count(), undoCountBefore + 1);
    QVERIFY(andGate->isSelected());
}

void TestQuickElementEditor::testBlobRenameAppliesUndoably()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic = placeEmbeddedTestIC(canvas, m_fixtureDir, "rename_editor_test");
    QVERIFY(ic);

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isBlobNameVisible());
    QCOMPARE(editor.blobName(), QString("rename_editor_test"));

    editor.commitBlobRename("renamed_editor_test");

    QCOMPARE(ic->blobName(), QString("renamed_editor_test"));
    QVERIFY(canvas.icRegistry()->hasBlob("renamed_editor_test"));
    QVERIFY(!canvas.icRegistry()->hasBlob("rename_editor_test"));

    canvas.undoStack()->undo();
    QCOMPARE(ic->blobName(), QString("rename_editor_test"));
}

void TestQuickElementEditor::testBlobRenameRejectsDuplicateName()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic1 = placeEmbeddedTestIC(canvas, m_fixtureDir, "editor_dup_a", {100, 100});
    auto *ic2 = placeEmbeddedTestIC(canvas, m_fixtureDir, "editor_dup_b", {300, 100});
    QVERIFY(ic1);
    QVERIFY(ic2);

    QuickElementEditor editor;
    selectOnly(canvas, editor, ic2);

    ScopedDialogStub guard;
    editor.commitBlobRename("editor_dup_a"); // collides with ic1's blob name

    QCOMPARE(guard.stub.choiceCallCount, 1);
    QCOMPARE(ic2->blobName(), QString("editor_dup_b")); // unchanged
}

void TestQuickElementEditor::testOpenTruthTableBuildsRowsAndEmitsSignal()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *table = ElementFactory::buildElement(ElementType::TruthTable);
    canvas.receiveCommand(new CanvasAddItemsCommand({table}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.isTruthTableVisible());

    QSignalSpy changedSpy(&editor, &QuickElementEditor::truthTableChanged);
    QSignalSpy requestedSpy(&editor, &QuickElementEditor::truthTableRequested);

    editor.openTruthTable();

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(requestedSpy.count(), 1);
    QCOMPARE(editor.truthTableInputCount(), table->inputSize());
    QCOMPARE(editor.truthTableRows().size(), 1 << table->inputSize());
    QCOMPARE(editor.truthTableColumnLabels().size(), table->inputSize() + table->outputSize());
}

void TestQuickElementEditor::testToggleTruthTableCellPushesUndoableCommand()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *table = ElementFactory::buildElement(ElementType::TruthTable);
    canvas.receiveCommand(new CanvasAddItemsCommand({table}, &canvas));
    auto *truthTable = qobject_cast<TruthTable *>(table);
    QVERIFY(truthTable);

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    editor.openTruthTable();

    const int outputColumn = truthTable->inputSize(); // first output column
    const QString before = editor.truthTableRows().first().cells().at(outputColumn);

    editor.toggleTruthTableCell(0, outputColumn);

    const QString after = editor.truthTableRows().first().cells().at(outputColumn);
    QVERIFY(after != before);

    canvas.undoStack()->undo();
    editor.openTruthTable(); // rebuild rows from the now-reverted table
    QCOMPARE(editor.truthTableRows().first().cells().at(outputColumn), before);
}
