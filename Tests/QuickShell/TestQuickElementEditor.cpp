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

void TestQuickElementEditor::testDivergentLabelShowsPlaceholderUntilEdited()
{
    // Mirrors applyCapabilitiesToUi()'s m_manyLabels placeholder: a multi-selection with
    // different labels shows a "<Many ...>" placeholder instead of silently displaying the
    // first element's label as though every selected element already shared it.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *led1 = ElementFactory::buildElement(ElementType::Led);
    auto *led2 = ElementFactory::buildElement(ElementType::Led);
    led1->setLabel("LED_A");
    led2->setLabel("LED_B");
    canvas.receiveCommand(new CanvasAddItemsCommand({led1, led2}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.hasSelection());
    QCOMPARE(editor.label(), QString("<Many labels>"));

    // Editing overwrites the placeholder and applies to every selected element, undoably --
    // apply()'s dirty-tracking treats a touched divergent field exactly like an ordinary edit.
    editor.setLabel("Unified");
    QCOMPARE(editor.label(), QString("Unified"));
    QCOMPARE(led1->label(), QString("Unified"));
    QCOMPARE(led2->label(), QString("Unified"));

    canvas.undoStack()->undo();
    QCOMPARE(led1->label(), QString("LED_A"));
    QCOMPARE(led2->label(), QString("LED_B"));
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

void TestQuickElementEditor::testDivergentTriggerShowsPlaceholderUntilEdited()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw2 = ElementFactory::buildElement(ElementType::InputSwitch);
    sw1->setTrigger(QKeySequence(Qt::Key_A));
    sw2->setTrigger(QKeySequence(Qt::Key_B));
    canvas.receiveCommand(new CanvasAddItemsCommand({sw1, sw2}, &canvas));

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(editor.hasSelection());
    QCOMPARE(editor.trigger(), QString("<Many triggers>"));

    editor.setTrigger("c");
    QCOMPARE(editor.trigger(), QString("C"));
    QCOMPARE(sw1->trigger(), QKeySequence(Qt::Key_C));
    QCOMPARE(sw2->trigger(), QKeySequence(Qt::Key_C));
}

void TestQuickElementEditor::testDelaySetAppliesUndoably()
{
    // Only Clock has ElementConstraints::hasDelay == true (see this class's header comment),
    // so this test exercises Clock rather than Node.
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

void TestQuickElementEditor::testWirelessModeAllowsUniqueTxLabelRename()
{
    // apply()'s duplicate-Tx scan runs on every apply() for any wireless-capable selection (not
    // just when the mode itself changes); this exercises its "other == elm" self-skip: the node
    // being renamed is scanned against every *other* Tx node's real label, never against its own.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *nodeA = ElementFactory::buildElement(ElementType::Node);
    auto *nodeB = ElementFactory::buildElement(ElementType::Node);
    nodeB->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({nodeA}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({nodeB}, &canvas));

    QuickElementEditor editor;
    selectOnly(canvas, editor, nodeA);
    editor.setLabel("CLOCK");
    editor.setWirelessMode(1); // Tx

    selectOnly(canvas, editor, nodeB);
    editor.setLabel("DATA");
    editor.setWirelessMode(1); // Tx

    selectOnly(canvas, editor, nodeA);
    ScopedDialogStub guard;
    editor.setLabel("BUS"); // unique -- no collision with nodeB's "DATA" or its own old label

    QCOMPARE(guard.stub.choiceCallCount, 0);
    QCOMPARE(nodeA->label(), QString("BUS"));
}

void TestQuickElementEditor::testWirelessModeAllowsRxNodeToShareLabelWithTx()
{
    // The duplicate-label guard in apply() only ever fires for a Tx *candidate*
    // (`candidateMode != WirelessMode::Tx` short-circuits the whole check), so an Rx node
    // sharing a label with an existing Tx node must be allowed.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode = ElementFactory::buildElement(ElementType::Node);
    rxNode->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({txNode}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({rxNode}, &canvas));

    QuickElementEditor editor;
    selectOnly(canvas, editor, txNode);
    editor.setLabel("SDA");
    editor.setWirelessMode(1); // Tx

    selectOnly(canvas, editor, rxNode);
    editor.setLabel("SDA_OLD");
    editor.setWirelessMode(2); // Rx

    ScopedDialogStub guard;
    editor.setLabel("SDA"); // same label as the Tx node -- allowed, this node is Rx

    QCOMPARE(guard.stub.choiceCallCount, 0);
    QCOMPARE(rxNode->label(), QString("SDA"));
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
        if (static_cast<ElementType>(candidate.toMap().value("type").toInt()) == ElementType::Or) {
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
        QVERIFY2(static_cast<ElementType>(candidate.toMap().value("type").toInt()) != ElementType::And,
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

// ===========================================================================
// Tab/Shift+Tab element cycling (mirrors master's TestElementTabNavigator)
// ===========================================================================

void TestQuickElementEditor::testReadingOrderIsRowMajor()
{
    // Two rows of two elements. Reading order must be row-major (top row left-to-right, then
    // bottom row left-to-right): a, b, c, d -- X as the primary sort key would wrongly return
    // column-major a, c, b, d.
    auto *a = ElementFactory::buildElement(ElementType::And);
    auto *b = ElementFactory::buildElement(ElementType::And);
    auto *c = ElementFactory::buildElement(ElementType::And);
    auto *d = ElementFactory::buildElement(ElementType::And);
    a->setPos(0, 0);     // top-left
    b->setPos(100, 0);   // top-right
    c->setPos(0, 100);   // bottom-left
    d->setPos(100, 100); // bottom-right

    const QList<GraphicElement *> scrambled = {d, b, c, a};
    const QList<GraphicElement *> expected = {a, b, c, d};
    QCOMPARE(QuickElementEditor::readingOrder(scrambled), expected);

    qDeleteAll(scrambled);
}

void TestQuickElementEditor::testReadingOrderTieBreaksLeftToRight()
{
    auto *left = ElementFactory::buildElement(ElementType::And);
    auto *middle = ElementFactory::buildElement(ElementType::And);
    auto *right = ElementFactory::buildElement(ElementType::And);
    left->setPos(0, 50);
    middle->setPos(100, 50);
    right->setPos(200, 50);

    const QList<GraphicElement *> scrambled = {right, left, middle};
    const QList<GraphicElement *> expected = {left, middle, right};
    QCOMPARE(QuickElementEditor::readingOrder(scrambled), expected);

    qDeleteAll(scrambled);
}

void TestQuickElementEditor::testReadingOrderEmptyAndSingle()
{
    QVERIFY(QuickElementEditor::readingOrder({}).isEmpty());

    auto *only = ElementFactory::buildElement(ElementType::And);
    only->setPos(42, 42);
    const QList<GraphicElement *> one = {only};
    QCOMPARE(QuickElementEditor::readingOrder(one), one);

    delete only;
}

void TestQuickElementEditor::testCycleSelectionTabAdvancesToNextElement()
{
    // Tab, while a single element is selected, advances the selection to the next element in
    // reading order that also exposes the requested field.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *left = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *right = ElementFactory::buildElement(ElementType::InputSwitch);
    left->setPos(0, 0);
    right->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({left, right}, &canvas));

    QuickElementEditor editor;
    selectOnly(canvas, editor, left);
    QVERIFY(editor.isLabelVisible());

    editor.cycleSelection(/*forward=*/true, "label");

    QVERIFY(!left->isSelected());
    QVERIFY(right->isSelected());
}

void TestQuickElementEditor::testCycleSelectionBacktabAdvancesToPreviousElement()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *left = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *right = ElementFactory::buildElement(ElementType::InputSwitch);
    left->setPos(0, 0);
    right->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({left, right}, &canvas));

    QuickElementEditor editor;
    selectOnly(canvas, editor, right);

    editor.cycleSelection(/*forward=*/false, "label");

    QVERIFY(!right->isSelected());
    QVERIFY(left->isSelected());
}

void TestQuickElementEditor::testCycleSelectionWrapsAround()
{
    // With only two elements, Tab from the last one in reading order wraps back to the first.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *left = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *right = ElementFactory::buildElement(ElementType::InputSwitch);
    left->setPos(0, 0);
    right->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({left, right}, &canvas));

    QuickElementEditor editor;
    selectOnly(canvas, editor, right);

    editor.cycleSelection(/*forward=*/true, "label");

    QVERIFY(!right->isSelected());
    QVERIFY(left->isSelected());
}

void TestQuickElementEditor::testCycleSelectionIgnoresNonSingleSelection()
{
    // Cycling only makes sense while exactly one element is being edited; with zero or two+
    // elements selected, cycleSelection() must no-op rather than acting on an arbitrary member.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *left = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *right = ElementFactory::buildElement(ElementType::InputSwitch);
    left->setPos(0, 0);
    right->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({left, right}, &canvas)); // both auto-selected

    QuickElementEditor editor;
    editor.setCanvas(&canvas);
    QVERIFY(left->isSelected());
    QVERIFY(right->isSelected());

    editor.cycleSelection(/*forward=*/true, "label");

    QVERIFY(left->isSelected());
    QVERIFY(right->isSelected());
}

void TestQuickElementEditor::testCycleSelectionRevertsWhenNoOtherElementHasField()
{
    // With only two elements, if the other one doesn't support the currently-focused field at
    // all (And has no label, unlike InputSwitch), cycling all the way back around without ever
    // finding a match must leave the original element selected.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    inputSwitch->setPos(0, 0);
    andGate->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({inputSwitch, andGate}, &canvas));

    QuickElementEditor editor;
    selectOnly(canvas, editor, inputSwitch);
    QVERIFY(editor.isLabelVisible());
    QVERIFY(!andGate->hasLabel());

    editor.cycleSelection(/*forward=*/true, "label");

    QVERIFY(inputSwitch->isSelected());
    QVERIFY(!andGate->isSelected());
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
