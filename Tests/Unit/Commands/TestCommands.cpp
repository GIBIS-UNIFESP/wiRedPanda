// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Commands/TestCommands.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Commands.h"
#include "Tests/Common/TestUtils.h"

void TestCommands::testAddItemsCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Add a single element
    QList<QGraphicsItem *> items{new And()};
    scene->receiveCommand(new AddItemsCommand(items, scene));

    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(undoStack->count(), 1);
    QVERIFY(undoStack->canUndo());
    QVERIFY(!undoStack->canRedo());

    // Undo should remove it
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 0);
    QVERIFY(undoStack->canRedo());

    // Redo should restore it
    undoStack->redo();
    QCOMPARE(scene->elements().size(), 1);
}

void TestCommands::testDeleteItemsCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // First add elements
    QList<QGraphicsItem *> items{new And(), new Or()};
    scene->receiveCommand(new AddItemsCommand(items, scene));
    QCOMPARE(scene->elements().size(), 2);

    // Delete them
    scene->receiveCommand(new DeleteItemsCommand(scene->items(), scene));
    QCOMPARE(scene->elements().size(), 0);

    // Undo should restore them
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 2);

    // Redo should delete again
    undoStack->redo();
    QCOMPARE(scene->elements().size(), 0);
}

void TestCommands::testAddDeleteUndoRedo()
{
    // This is the original test - kept for regression testing
    QList<QGraphicsItem *> items{new And(), new And(), new And(), new And()};

    WorkSpace workspace;
    auto *scene = workspace.scene();
    QVERIFY(scene != nullptr);
    auto *undoStack = scene->undoStack();
    QVERIFY(undoStack != nullptr);
    undoStack->setUndoLimit(1);
    scene->receiveCommand(new AddItemsCommand(items, scene));

    QCOMPARE(scene->elements().size(), items.size());

    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    scene->receiveCommand(new DeleteItemsCommand(scene->items(), scene));
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();

    QCOMPARE(scene->elements().size(), 0);
    QCOMPARE(undoStack->index(), 1);
}

void TestCommands::testRotateCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Add an element
    auto *andGate = new And();
    QList<QGraphicsItem *> items{andGate};
    scene->receiveCommand(new AddItemsCommand(items, scene));

    // Get initial rotation
    const int initialRotation = static_cast<int>(andGate->rotation());

    // Rotate 90 degrees
    QList<GraphicElement *> elements = scene->elements().toList();
    scene->receiveCommand(new RotateCommand(elements, 90, scene));

    QCOMPARE(static_cast<int>(andGate->rotation()), (initialRotation + 90) % 360);

    // Undo should restore original rotation
    undoStack->undo();
    QCOMPARE(static_cast<int>(andGate->rotation()), initialRotation);

    // Redo should rotate again
    undoStack->redo();
    QCOMPARE(static_cast<int>(andGate->rotation()), (initialRotation + 90) % 360);
}

void TestCommands::testMoveCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Add an element
    auto *andGate = new And();
    QList<QGraphicsItem *> items{andGate};
    scene->receiveCommand(new AddItemsCommand(items, scene));

    // Get position AFTER adding to scene
    const QPointF posAfterAdd = andGate->pos();

    // Store old position and move the element
    QList<GraphicElement *> elements = scene->elements().toList();
    QList<QPointF> oldPositions{posAfterAdd};

    // Move the element to a new position (relative offset)
    const QPointF offset(100, 100);
    andGate->setPos(posAfterAdd + offset);

    // Capture position after manual move (this becomes newPos in the command)
    const QPointF posAfterMove = andGate->pos();
    scene->receiveCommand(new MoveCommand(elements, oldPositions, scene));

    // Position should still be at moved location
    QCOMPARE(andGate->pos(), posAfterMove);

    // Undo should restore original position
    undoStack->undo();
    QCOMPARE(andGate->pos(), posAfterAdd);

    // Redo should move again
    undoStack->redo();
    QCOMPARE(andGate->pos(), posAfterMove);
}

void TestCommands::testFlipCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Add elements
    auto *gate1 = new And();
    auto *gate2 = new And();

    QList<QGraphicsItem *> items{gate1, gate2};
    scene->receiveCommand(new AddItemsCommand(items, scene));

    // Set positions AFTER adding to scene
    gate1->setPos(100, 100);
    gate2->setPos(200, 100);

    const QPointF pos1Before = gate1->pos();
    const QPointF pos2Before = gate2->pos();

    // Flip horizontally (axis = 0)
    QList<GraphicElement *> elements = scene->elements().toList();
    scene->receiveCommand(new FlipCommand(elements, 0, scene));

    // After horizontal flip, x positions should be swapped relative to center
    // The center is at x=150, so gate1 (at 100) should move to 200, gate2 (at 200) to 100
    const QPointF pos1After = gate1->pos();
    const QPointF pos2After = gate2->pos();

    // Verify that positions actually changed
    QVERIFY2(pos1After != pos1Before || pos2After != pos2Before,
             "Flip command should have changed element positions");

    // Undo should restore original positions
    undoStack->undo();
    QCOMPARE(gate1->pos(), pos1Before);
    QCOMPARE(gate2->pos(), pos2Before);
}

void TestCommands::testChangeInputSizeCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Add an AND gate (default 2 inputs)
    auto *andGate = new And();
    QList<QGraphicsItem *> items{andGate};
    scene->receiveCommand(new AddItemsCommand(items, scene));

    const int originalInputSize = andGate->inputSize();
    QCOMPARE(originalInputSize, 2);

    // Change to 4 inputs
    QList<GraphicElement *> elements{andGate};
    scene->receiveCommand(new ChangePortSizeCommand(elements, 4, scene, true));

    QCOMPARE(andGate->inputSize(), 4);

    // Undo should restore original size
    undoStack->undo();
    QCOMPARE(andGate->inputSize(), originalInputSize);

    // Redo should change size again
    undoStack->redo();
    QCOMPARE(andGate->inputSize(), 4);
}

void TestCommands::testConnectionPreservation()
{
    // Test that undo/redo properly restores connections
    // This was a bug that was fixed (commit 6f37fbd4)
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Create a simple circuit: Switch -> AND -> LED
    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *andGate = new And();
    auto *led = new Led();

    sw1->setPos(0, 0);
    sw2->setPos(0, 100);
    andGate->setPos(100, 50);
    led->setPos(200, 50);

    QList<QGraphicsItem *> elements{sw1, sw2, andGate, led};
    scene->receiveCommand(new AddItemsCommand(elements, scene));

    // Create connections
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(sw1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(sw2->outputPort());
    conn2->setEndPort(andGate->inputPort(1));

    auto *conn3 = new QNEConnection();
    conn3->setStartPort(andGate->outputPort());
    conn3->setEndPort(led->inputPort());

    QList<QGraphicsItem *> connections{conn1, conn2, conn3};
    scene->receiveCommand(new AddItemsCommand(connections, scene));

    // Verify connections exist
    QCOMPARE(TestUtils::countConnections(scene), 3);

    // Delete everything
    scene->receiveCommand(new DeleteItemsCommand(scene->items(), scene));
    QCOMPARE(scene->elements().size(), 0);
    QCOMPARE(TestUtils::countConnections(scene), 0);

    // Undo delete - connections should be restored
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 4);
    QCOMPARE(TestUtils::countConnections(scene), 3);

    // Verify connections are properly connected
    const auto restoredConnections = TestUtils::getConnections(scene);
    for (auto *conn : std::as_const(restoredConnections)) {
        QVERIFY2(conn->startPort() != nullptr, "Connection missing start port after undo");
        QVERIFY2(conn->endPort() != nullptr, "Connection missing end port after undo");
    }
}

void TestCommands::testUndoEmptyStack()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Verify undo on empty stack doesn't crash
    QVERIFY(!undoStack->canUndo());
    undoStack->undo(); // Should be safe no-op
    QCOMPARE(scene->elements().size(), 0);
}

void TestCommands::testRedoEmptyStack()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Verify redo on empty stack doesn't crash
    QVERIFY(!undoStack->canRedo());
    undoStack->redo(); // Should be safe no-op
    QCOMPARE(scene->elements().size(), 0);
}

void TestCommands::testUndoLimit()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Set undo limit to 2
    undoStack->setUndoLimit(2);

    // Add 3 elements one at a time
    scene->receiveCommand(new AddItemsCommand({new And()}, scene));
    scene->receiveCommand(new AddItemsCommand({new And()}, scene));
    scene->receiveCommand(new AddItemsCommand({new And()}, scene));

    QCOMPARE(scene->elements().size(), 3);

    // Should only be able to undo 2 operations due to limit
    QCOMPARE(undoStack->count(), 2);

    undoStack->undo();
    undoStack->undo();
    QVERIFY(!undoStack->canUndo());

    // First element should still exist (its add was pushed out of history)
    QCOMPARE(scene->elements().size(), 1);
}

void TestCommands::testChangeOutputSizeCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Add a TruthTable element (supports 1-8 outputs)
    auto *truthTable = new TruthTable();
    QList<QGraphicsItem *> items{truthTable};
    scene->receiveCommand(new AddItemsCommand(items, scene));

    const int originalOutputSize = truthTable->outputSize();
    QCOMPARE(originalOutputSize, 1);

    // Change to 4 outputs
    QList<GraphicElement *> elements{truthTable};
    scene->receiveCommand(new ChangePortSizeCommand(elements, 4, scene, false));

    QCOMPARE(truthTable->outputSize(), 4);

    // Undo should restore original size
    undoStack->undo();
    QCOMPARE(truthTable->outputSize(), originalOutputSize);

    // Redo should change size again
    undoStack->redo();
    QCOMPARE(truthTable->outputSize(), 4);
}

void TestCommands::testMorphCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Add an AND gate
    auto *andGate = new And();
    QList<QGraphicsItem *> items{andGate};
    scene->receiveCommand(new AddItemsCommand(items, scene));

    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements().first()->elementType(), ElementType::And);

    // Morph to OR gate
    QList<GraphicElement *> elements = scene->elements().toList();
    scene->receiveCommand(new MorphCommand(elements, ElementType::Or, scene));

    // After morph, should have 1 element of type OR
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements().first()->elementType(), ElementType::Or);

    // Undo should restore to AND gate
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements().first()->elementType(), ElementType::And);

    // Redo should morph back to OR
    undoStack->redo();
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements().first()->elementType(), ElementType::Or);
}

void TestCommands::testSplitCommandRedoThrowsBeforeAllocation()
{
    // Pre-fix SplitCommand::redo allocated conn2 + node BEFORE checking that
    // the upstream items still existed. When the check failed, those two
    // freshly-allocated objects were leaked. Reordering the check to run
    // first means redo throws without ever calling new.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    sw->setPos(0, 0);
    led->setPos(200, 0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{sw, led}, scene));

    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));

    scene->receiveCommand(new SplitCommand(conn, QPointF(100, 0), scene));
    QCOMPARE(scene->elements().size(), 3);

    undoStack->undo();
    QCOMPARE(scene->elements().size(), 2);

    // Make the upstream lookup fail. forgetItemId drops the registry entry
    // so findElm(m_elm1Id) returns nullptr without freeing sw itself.
    scene->forgetItemId(sw->id());

    const int elementsBefore = scene->elements().size();
    const int connectionsBefore = TestUtils::countConnections(scene);
    QVERIFY_EXCEPTION_THROWN(undoStack->redo(), std::exception);

    // No conn2 / node should have been added — they're never allocated on the
    // throw path now, and even on the old path they were leaked rather than
    // attached, so scene counts staying flat is the load-bearing invariant.
    QCOMPARE(scene->elements().size(), elementsBefore);
    QCOMPARE(TestUtils::countConnections(scene), connectionsBefore);
}

void TestCommands::testSplitCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Create a simple circuit: Switch -> LED with one connection
    auto *sw = new InputSwitch();
    auto *led = new Led();

    sw->setPos(0, 0);
    led->setPos(200, 0);

    QList<QGraphicsItem *> elements{sw, led};
    scene->receiveCommand(new AddItemsCommand(elements, scene));

    // Create connection between switch and LED
    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());

    QList<QGraphicsItem *> connections{conn};
    scene->receiveCommand(new AddItemsCommand(connections, scene));

    // Verify initial state: 2 elements, 1 connection
    QCOMPARE(scene->elements().size(), 2);
    QCOMPARE(TestUtils::countConnections(scene), 1);

    // Split the connection in the middle
    QPointF splitPos(100, 0);
    scene->receiveCommand(new SplitCommand(conn, splitPos, scene));

    // After split: 3 elements (switch, led, node), 2 connections
    QCOMPARE(scene->elements().size(), 3);
    QCOMPARE(TestUtils::countConnections(scene), 2);

    // Check that a Node was added
    bool hasNode = false;
    for (auto *elm : scene->elements()) {
        if (elm->elementType() == ElementType::Node) {
            hasNode = true;
            break;
        }
    }
    QVERIFY2(hasNode, "Split should create a Node element");

    // Undo should restore original state
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 2);
    QCOMPARE(TestUtils::countConnections(scene), 1);

    // Redo should split again
    undoStack->redo();
    QCOMPARE(scene->elements().size(), 3);
    QCOMPARE(TestUtils::countConnections(scene), 2);
}

void TestCommands::testUpdateCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Add an AND gate with a label
    auto *andGate = new And();
    QList<QGraphicsItem *> items{andGate};
    scene->receiveCommand(new AddItemsCommand(items, scene));

    // Set initial label
    const QString originalLabel = "OriginalLabel";
    andGate->setLabel(originalLabel);
    QCOMPARE(andGate->label(), originalLabel);

    // Capture old state before modification
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    andGate->save(stream);

    // Modify the element (change label)
    const QString newLabel = "NewLabel";
    andGate->setLabel(newLabel);
    QCOMPARE(andGate->label(), newLabel);

    // Create update command
    QList<GraphicElement *> elements{andGate};
    scene->receiveCommand(new UpdateCommand(elements, oldData, scene));

    // Verify label is still the new one
    QCOMPARE(andGate->label(), newLabel);

    // Undo should restore original label
    undoStack->undo();
    QCOMPARE(andGate->label(), originalLabel);

    // Redo should apply new label again
    undoStack->redo();
    QCOMPARE(andGate->label(), newLabel);
}

