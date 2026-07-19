// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasCommands.h"

#include <QDataStream>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

/// Builds two elements already wired output(0)->input(inputIndex) and adds both to \a canvas
/// via a single CanvasAddItemsCommand (which auto-discovers the connection through port
/// traversal -- see CanvasCommandUtils::loadList()'s doc comment).
void addWiredPair(CanvasItem &canvas, GraphicElement *source, GraphicElement *sink, int inputIndex = 0)
{
    auto *conn = new Connection();
    conn->setStartPort(source->outputPort(0));
    conn->setEndPort(sink->inputPort(inputIndex));
    canvas.receiveCommand(new CanvasAddItemsCommand({source, sink}, &canvas));
}

/// CanvasCommandUtils::addItems() selects every element it adds (mirrors production's own
/// "newly added elements start selected" UX), so a CanvasAddItemsCommand covering several
/// elements at once leaves all of them selected -- selection-driven canvas methods
/// (deleteSelected(), morphSelectionTo()) need exactly one target selected, not the whole batch.
void selectOnly(CanvasItem &canvas, GraphicElement *target)
{
    for (auto *elm : canvas.elements()) {
        elm->setSelected(elm == target);
    }
}

} // namespace

void TestCanvasCommands::testAddItemsCommandWithConnectionsUndoRedo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    addWiredPair(canvas, sw, led);

    QCOMPARE(canvas.elements().size(), 2);
    QCOMPARE(canvas.connections().size(), 1);
    QCOMPARE(led->inputPort(0)->connections().size(), 1);

    canvas.undoStack()->undo();
    QCOMPARE(canvas.elements().size(), 0);
    QCOMPARE(canvas.connections().size(), 0);

    canvas.undoStack()->redo();
    QCOMPARE(canvas.elements().size(), 2);
    QCOMPARE(canvas.connections().size(), 1);
}

void TestCanvasCommands::testDeleteSelectedRemovesConnectedWireUndoRestoresIt()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    addWiredPair(canvas, sw, led);

    selectOnly(canvas, sw);
    canvas.deleteSelected();

    QCOMPARE(canvas.elements().size(), 1);
    QCOMPARE(canvas.connections().size(), 0);
    QCOMPARE(led->inputPort(0)->connections().size(), 0);

    canvas.undoStack()->undo();
    QCOMPARE(canvas.elements().size(), 2);
    QCOMPARE(canvas.connections().size(), 1);
}

void TestCanvasCommands::testMoveCommandUndoRedo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));

    const QPointF oldPos = sw->pos();
    const QPointF newPos = oldPos + QPointF(80, 40);

    sw->setPos(newPos);
    canvas.receiveCommand(new CanvasMoveCommand({sw}, {oldPos}, &canvas));
    QCOMPARE(sw->pos(), newPos);

    canvas.undoStack()->undo();
    QCOMPARE(sw->pos(), oldPos);

    canvas.undoStack()->redo();
    QCOMPARE(sw->pos(), newPos);
}

void TestCanvasCommands::testMoveMultipleElementsTogetherUndoRedo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw2 = ElementFactory::buildElement(ElementType::InputSwitch);
    sw2->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw1, sw2}, &canvas));

    const QPointF old1 = sw1->pos();
    const QPointF old2 = sw2->pos();
    const QPointF delta(30, 30);

    sw1->setPos(old1 + delta);
    sw2->setPos(old2 + delta);
    canvas.receiveCommand(new CanvasMoveCommand({sw1, sw2}, {old1, old2}, &canvas));

    canvas.undoStack()->undo();
    QCOMPARE(sw1->pos(), old1);
    QCOMPARE(sw2->pos(), old2);

    canvas.undoStack()->redo();
    QCOMPARE(sw1->pos(), old1 + delta);
    QCOMPARE(sw2->pos(), old2 + delta);
}

void TestCanvasCommands::testRotateRightAccumulatesUndoUnwindsOneStep()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    elm->setSelected(true);
    canvas.rotateRight();
    QCOMPARE(elm->rotation(), 90.0);

    canvas.rotateRight();
    QCOMPARE(elm->rotation(), 180.0);

    canvas.undoStack()->undo();
    QCOMPARE(elm->rotation(), 90.0);

    canvas.undoStack()->undo();
    QCOMPARE(elm->rotation(), 0.0);
}

void TestCanvasCommands::testFlipHorizontalTwiceIsIdentity()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    const QPointF originalPos = elm->pos();
    QVERIFY(!elm->isFlippedX());

    elm->setSelected(true);
    canvas.flipHorizontally();
    QVERIFY(elm->isFlippedX());

    canvas.flipHorizontally();
    QCOMPARE(elm->pos(), originalPos);
    QVERIFY(!elm->isFlippedX());
}

void TestCanvasCommands::testFlipVerticalUndoRedo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    QVERIFY(!elm->isFlippedY());

    elm->setSelected(true);
    canvas.flipVertically();
    QVERIFY(elm->isFlippedY());

    canvas.undoStack()->undo();
    QVERIFY(!elm->isFlippedY());

    canvas.undoStack()->redo();
    QVERIFY(elm->isFlippedY());
}

void TestCanvasCommands::testUpdateCommandLabelUndoRedo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *elm = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    elm->setLabel(QStringLiteral("Original"));

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});

    elm->setLabel(QStringLiteral("Renamed"));
    canvas.receiveCommand(new CanvasUpdateCommand({elm}, oldData, &canvas));
    QCOMPARE(elm->label(), QStringLiteral("Renamed"));

    canvas.undoStack()->undo();
    QCOMPARE(elm->label(), QStringLiteral("Original"));

    canvas.undoStack()->redo();
    QCOMPARE(elm->label(), QStringLiteral("Renamed"));
}

void TestCanvasCommands::testChangePortSizeIncreaseInputSize()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *gate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({gate}, &canvas));

    const int oldSize = gate->inputSize();
    const int newSize = oldSize + 1;

    canvas.receiveCommand(new CanvasChangePortSizeCommand({gate}, newSize, &canvas, /*isInput=*/true));
    QCOMPARE(gate->inputSize(), newSize);

    canvas.undoStack()->undo();
    QCOMPARE(gate->inputSize(), oldSize);

    canvas.undoStack()->redo();
    QCOMPARE(gate->inputSize(), newSize);
}

void TestCanvasCommands::testChangePortSizeDecreaseRemovesConnectionRestoredOnUndo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    // And's own minInputSize is 2 (see And.cpp), so a shrink test needs to start above that
    // floor: grow to 3 first, wire all three, then shrink back to 2 (still >= the floor) to
    // exercise a real port-drop rather than an outright rejected size.
    auto *gate = ElementFactory::buildElement(ElementType::And);
    auto *sw0 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw2 = ElementFactory::buildElement(ElementType::InputSwitch);
    QCOMPARE(gate->inputSize(), 2);

    auto *conn0 = new Connection();
    conn0->setStartPort(sw0->outputPort(0));
    conn0->setEndPort(gate->inputPort(0));
    auto *conn1 = new Connection();
    conn1->setStartPort(sw1->outputPort(0));
    conn1->setEndPort(gate->inputPort(1));
    canvas.receiveCommand(new CanvasAddItemsCommand({gate, sw0, sw1, sw2}, &canvas));

    canvas.receiveCommand(new CanvasChangePortSizeCommand({gate}, 3, &canvas, /*isInput=*/true));
    QCOMPARE(gate->inputSize(), 3);

    auto *conn2 = new Connection();
    conn2->setStartPort(sw2->outputPort(0));
    conn2->setEndPort(gate->inputPort(2));
    canvas.receiveCommand(new CanvasAddItemsCommand({conn2}, &canvas));

    const int survivingConnId = conn1->id();
    QVERIFY(!sw1->outputPort(0)->connections().isEmpty());
    QVERIFY(!sw2->outputPort(0)->connections().isEmpty());

    canvas.receiveCommand(new CanvasChangePortSizeCommand({gate}, 2, &canvas, /*isInput=*/true));
    QCOMPARE(gate->inputSize(), 2);
    QVERIFY(sw2->outputPort(0)->connections().isEmpty()); // dropped: was on the trimmed port
    QVERIFY(!sw1->outputPort(0)->connections().isEmpty()); // kept: still within the new size

    canvas.undoStack()->undo();
    QCOMPARE(gate->inputSize(), 3);
    QCOMPARE(sw1->outputPort(0)->connections().size(), 1);
    QCOMPARE(sw1->outputPort(0)->connections().constFirst()->id(), survivingConnId);
    QVERIFY(!sw2->outputPort(0)->connections().isEmpty());
}

void TestCanvasCommands::testMorphPreservesConnections()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *gate = ElementFactory::buildElement(ElementType::And);
    auto *sw0 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    QCOMPARE(gate->inputSize(), 2);

    auto *conn0 = new Connection();
    conn0->setStartPort(sw0->outputPort(0));
    conn0->setEndPort(gate->inputPort(0));
    auto *conn1 = new Connection();
    conn1->setStartPort(sw1->outputPort(0));
    conn1->setEndPort(gate->inputPort(1));
    auto *connOut = new Connection();
    connOut->setStartPort(gate->outputPort(0));
    connOut->setEndPort(led->inputPort(0));
    canvas.receiveCommand(new CanvasAddItemsCommand({gate, sw0, sw1, led}, &canvas));

    selectOnly(canvas, gate);
    canvas.morphSelectionTo(ElementType::Or);

    // The original And instance was replaced -- find the surviving Or element.
    GraphicElement *morphed = nullptr;
    for (auto *elm : canvas.elements()) {
        if (elm->elementType() == ElementType::Or) {
            morphed = elm;
        }
    }
    QVERIFY(morphed);
    QCOMPARE(morphed->inputSize(), 2);
    QVERIFY(!morphed->inputPort(0)->connections().isEmpty());
    QVERIFY(!morphed->inputPort(1)->connections().isEmpty());
    QVERIFY(!morphed->outputPort(0)->connections().isEmpty());

    canvas.undoStack()->undo();
    bool andRestored = false;
    for (auto *elm : canvas.elements()) {
        if (elm->elementType() == ElementType::And) {
            andRestored = true;
            QVERIFY(!elm->inputPort(0)->connections().isEmpty());
            QVERIFY(!elm->inputPort(1)->connections().isEmpty());
        }
    }
    QVERIFY(andRestored);
}

void TestCanvasCommands::testMorphToFewerPortsDropsConnectionRestoredOnUndo()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *gate = ElementFactory::buildElement(ElementType::And);
    auto *sw0 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    QCOMPARE(gate->inputSize(), 2);

    auto *conn0 = new Connection();
    conn0->setStartPort(sw0->outputPort(0));
    conn0->setEndPort(gate->inputPort(0));
    auto *conn1 = new Connection();
    conn1->setStartPort(sw1->outputPort(0));
    conn1->setEndPort(gate->inputPort(1));
    canvas.receiveCommand(new CanvasAddItemsCommand({gate, sw0, sw1}, &canvas));

    selectOnly(canvas, gate);
    canvas.morphSelectionTo(ElementType::Not); // 1 input port -- drops the sw1 connection

    GraphicElement *morphed = nullptr;
    for (auto *elm : canvas.elements()) {
        if (elm->elementType() == ElementType::Not) {
            morphed = elm;
        }
    }
    QVERIFY(morphed);
    QCOMPARE(morphed->inputSize(), 1);
    QVERIFY(sw1->outputPort(0)->connections().isEmpty());

    canvas.undoStack()->undo();
    bool andRestored = false;
    for (auto *elm : canvas.elements()) {
        if (elm->elementType() == ElementType::And) {
            andRestored = true;
        }
    }
    QVERIFY(andRestored);
    QVERIFY(!sw1->outputPort(0)->connections().isEmpty());
}

void TestCanvasCommands::testSplitCommandCreatesNodeUndoRestoresSingleConnection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(200, 0);
    addWiredPair(canvas, sw, led);

    QCOMPARE(canvas.elements().size(), 2);
    QCOMPARE(canvas.connections().size(), 1);
    Connection *original = canvas.connections().constFirst();

    const QPointF midpoint = (sw->pos() + led->pos()) / 2.0;
    canvas.receiveCommand(new CanvasSplitCommand(original, midpoint, &canvas));

    QCOMPARE(canvas.elements().size(), 3); // Node inserted
    QCOMPARE(canvas.connections().size(), 2);

    canvas.undoStack()->undo();
    QCOMPARE(canvas.elements().size(), 2);
    QCOMPARE(canvas.connections().size(), 1);
    QCOMPARE(led->inputPort(0)->connections().size(), 1);
}

void TestCanvasCommands::testUndoOnEmptyStackIsNoop()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    QVERIFY(!canvas.undoStack()->canUndo());
    canvas.undoStack()->undo(); // must not crash
    QCOMPARE(canvas.elements().size(), 0);
}

void TestCanvasCommands::testRedoOnEmptyStackIsNoop()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    QVERIFY(!canvas.undoStack()->canRedo());
    canvas.undoStack()->redo(); // must not crash
    QCOMPARE(canvas.elements().size(), 0);
}

void TestCanvasCommands::testNewOperationClearsRedoStack()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw1}, &canvas));
    canvas.undoStack()->undo();
    QVERIFY(canvas.undoStack()->canRedo());

    auto *sw2 = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw2}, &canvas));

    QVERIFY(!canvas.undoStack()->canRedo());
    QCOMPARE(canvas.elements().size(), 1);
    QCOMPARE(canvas.elements().first(), sw2);
}
