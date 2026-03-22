// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestSceneUndoredo.h"

#include <QDataStream>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoCommand>

#include "App/Element/ElementFactory.h"
#include "App/GlobalProperties.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "Tests/Common/TestUtils.h"

void TestSceneUndoredo::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    GlobalProperties::currentDir = m_tempDir.path();
}

void TestSceneUndoredo::testReceiveCommand()
{
    Scene scene;

    // Create a simple undo command
    auto *command = new QUndoCommand("Test Command");

    // Receive the command (should queue it)
    scene.receiveCommand(command);

    // Verify undo stack has the command
    QVERIFY(!scene.undoStack()->isClean());
}

void TestSceneUndoredo::testPasteUndoRedo()
{
    Scene scene;

    // Create and copy an element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    andGate->setSelected(true);

    // Copy
    scene.copyAction();

    // Paste
    scene.pasteAction();

    // Should have 2 elements now
    QCOMPARE(scene.elements().size(), 2);

    // Undo paste
    scene.undoStack()->undo();

    // Should have 1 element
    QCOMPARE(scene.elements().size(), 1);

    // Redo paste
    scene.undoStack()->redo();

    // Should have 2 elements again
    QCOMPARE(scene.elements().size(), 2);
}

void TestSceneUndoredo::testCutUndoRedo()
{
    Scene scene;

    // Create 2 elements
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1);

    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(100, 0);
    scene.addItem(and2);

    QCOMPARE(scene.elements().size(), 2);

    // Select first element and cut
    and1->setSelected(true);
    scene.cutAction();

    // Should have 1 element
    QCOMPARE(scene.elements().size(), 1);

    // Undo cut
    scene.undoStack()->undo();

    // Should have 2 elements
    QCOMPARE(scene.elements().size(), 2);

    // Redo cut
    scene.undoStack()->redo();

    // Should have 1 element
    QCOMPARE(scene.elements().size(), 1);
}

void TestSceneUndoredo::testMultiCommandSequence()
{
    Scene scene;

    // Create initial element and set up clipboard
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    andGate->setSelected(true);
    scene.copyAction();

    QCOMPARE(scene.elements().size(), 1);

    // Paste 3 times — each creates a real AddItemsCommand on the undo stack
    scene.pasteAction();
    QCOMPARE(scene.elements().size(), 2);
    scene.pasteAction();
    QCOMPARE(scene.elements().size(), 3);
    scene.pasteAction();
    QCOMPARE(scene.elements().size(), 4);

    // Undo all 3 pastes — each undo removes one pasted element
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 3);
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 2);
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 1);

    // Redo all 3 pastes — each redo restores one pasted element
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 2);
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 3);
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 4);
}

// ─── DeleteItemsCommand ────────────────────────────────────────────────────

void TestSceneUndoredo::testDeleteElementAndUndo()
{
    Scene scene;

    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elm);
    const int elmId = elm->id();
    QCOMPARE(scene.elements().size(), 1);

    elm->setSelected(true);
    scene.deleteAction();
    QCOMPARE(scene.elements().size(), 0);

    // Undo → element restored
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 1);
    QVERIFY(ElementFactory::itemById(elmId) != nullptr);

    // Redo → element deleted again
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 0);
}

void TestSceneUndoredo::testDeleteWithConnectionsRestored()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(andGate);
    scene.addItem(led);

    auto *conn = new QNEConnection();
    conn->setStartPort(andGate->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    scene.addItem(conn);

    QCOMPARE(scene.elements().size(), 2);
    QVERIFY(!andGate->outputPort(0)->connections().isEmpty());

    // Delete AND gate — its connection should also be removed
    andGate->setSelected(true);
    scene.deleteAction();
    QCOMPARE(scene.elements().size(), 1);
    // Connection is removed from scene (port has no connections)
    QVERIFY(led->inputPort(0)->connections().isEmpty());

    // Undo → AND gate and connection restored
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 2);
    // Connection is back — the LED's input port is connected again
    QVERIFY(!led->inputPort(0)->connections().isEmpty());

    // Redo → AND gate and connection gone again
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 1);
    QVERIFY(led->inputPort(0)->connections().isEmpty());
}

// ─── RotateCommand ────────────────────────────────────────────────────────

void TestSceneUndoredo::testRotateSingleElement()
{
    Scene scene;

    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elm);
    const int elmId = elm->id();
    QCOMPARE(elm->rotation(), 0.0);

    scene.undoStack()->push(new RotateCommand({elm}, 90, &scene));

    auto *after = dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId));
    QCOMPARE(after->rotation(), 90.0);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->rotation(), 0.0);
}

void TestSceneUndoredo::testRotateUndoRedo()
{
    Scene scene;

    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elm);
    const int elmId = elm->id();

    scene.undoStack()->push(new RotateCommand({elm}, 90, &scene));
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->rotation(), 90.0);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->rotation(), 0.0);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->rotation(), 90.0);
}

void TestSceneUndoredo::testRotateAccumulates()
{
    Scene scene;

    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elm);
    const int elmId = elm->id();

    // Two 90° rotations = 180° total
    scene.undoStack()->push(new RotateCommand({elm}, 90, &scene));
    elm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId));
    scene.undoStack()->push(new RotateCommand({elm}, 90, &scene));
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->rotation(), 180.0);

    // Undo second → 90°
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->rotation(), 90.0);

    // Undo first → 0°
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->rotation(), 0.0);
}

// ─── MoveCommand ──────────────────────────────────────────────────────────

void TestSceneUndoredo::testMoveSingleElement()
{
    Scene scene;

    // Use multiples of 8 (gridSize/2) to avoid grid-snap rounding
    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(0, 0);
    scene.addItem(elm);
    const int elmId = elm->id();
    const QPointF oldPos = elm->pos(); // (0,0)

    elm->setPos(96, 96); // 96 = 12 * 8, snaps cleanly
    const QPointF newPos = elm->pos(); // read back the snapped position

    scene.undoStack()->push(new MoveCommand({elm}, {oldPos}, &scene));

    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->pos(), newPos);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->pos(), oldPos);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(elmId))->pos(), newPos);
}

void TestSceneUndoredo::testMoveMultipleElements()
{
    Scene scene;

    // Use multiples of 8 to avoid grid-snap rounding
    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(48, 48); // 48 = 6 * 8
    scene.addItem(elm1);
    scene.addItem(elm2);
    const int id1 = elm1->id();
    const int id2 = elm2->id();
    const QPointF old1 = elm1->pos();
    const QPointF old2 = elm2->pos();

    elm1->setPos(192, 192); // 192 = 24 * 8
    elm2->setPos(240, 240); // 240 = 30 * 8
    const QPointF new1 = elm1->pos();
    const QPointF new2 = elm2->pos();

    scene.undoStack()->push(new MoveCommand({elm1, elm2}, {old1, old2}, &scene));

    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->pos(), new1);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->pos(), new2);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->pos(), old1);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->pos(), old2);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->pos(), new1);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->pos(), new2);
}

// ─── FlipCommand ──────────────────────────────────────────────────────────

void TestSceneUndoredo::testFlipHorizontalUndoRedo()
{
    Scene scene;

    // Use multiples of 8 to avoid grid-snap rounding; place elements, read back snapped positions
    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(96, 0); // 96 = 12 * 8
    scene.addItem(elm1);
    scene.addItem(elm2);
    const int id1 = elm1->id();
    const int id2 = elm2->id();
    const double x1 = elm1->pos().x(); // 0
    const double x2 = elm2->pos().x(); // 96

    // Horizontal flip (axis=0): minX=x1, maxX=x2
    // elm1 → x1+(x2-x1) = x2; elm2 → x1+(x2-x2) = x1
    scene.undoStack()->push(new FlipCommand({elm1, elm2}, 0, &scene));

    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->pos().x(), x2);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->pos().x(), x1);

    // undo() calls redo() — flip is involution for positions
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->pos().x(), x1);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->pos().x(), x2);

    // redo() → flip again
    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->pos().x(), x2);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->pos().x(), x1);
}

void TestSceneUndoredo::testFlipVerticalUndoRedo()
{
    Scene scene;

    // Use multiples of 8 to avoid grid-snap rounding
    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(0, 96); // 96 = 12 * 8
    scene.addItem(elm1);
    scene.addItem(elm2);
    const int id1 = elm1->id();
    const int id2 = elm2->id();
    const double y1 = elm1->pos().y(); // 0
    const double y2 = elm2->pos().y(); // 96

    // Vertical flip (axis=1): minY=y1, maxY=y2
    scene.undoStack()->push(new FlipCommand({elm1, elm2}, 1, &scene));

    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->pos().y(), y2);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->pos().y(), y1);

    // undo() calls redo() — flip back
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->pos().y(), y1);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->pos().y(), y2);
}

// ─── MorphCommand ─────────────────────────────────────────────────────────

void TestSceneUndoredo::testMorphTypeChange()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    const int id = andGate->id();

    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->elementType(), ElementType::And);

    // Push MorphCommand AND → OR; redo() is called immediately, andGate pointer becomes invalid
    scene.undoStack()->push(new MorphCommand({andGate}, ElementType::Or, &scene));

    auto *morphed = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
    QVERIFY(morphed != nullptr);
    QCOMPARE(morphed->elementType(), ElementType::Or);

    // Undo → back to AND
    scene.undoStack()->undo();
    auto *restored = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->elementType(), ElementType::And);

    // Redo → OR again
    scene.undoStack()->redo();
    auto *redone = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
    QVERIFY(redone != nullptr);
    QCOMPARE(redone->elementType(), ElementType::Or);
}

void TestSceneUndoredo::testMorphPreservesLabel()
{
    Scene scene;

    // InputSwitch and InputButton both have labels
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    sw->setLabel("testLabel");
    scene.addItem(sw);
    const int id = sw->id();

    scene.undoStack()->push(new MorphCommand({sw}, ElementType::InputButton, &scene));

    auto *morphed = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
    QVERIFY(morphed != nullptr);
    QCOMPARE(morphed->elementType(), ElementType::InputButton);
    QCOMPARE(morphed->label(), QString("testLabel"));

    // Undo → InputSwitch with label preserved
    scene.undoStack()->undo();
    auto *restored = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->elementType(), ElementType::InputSwitch);
    QCOMPARE(restored->label(), QString("testLabel"));
}

void TestSceneUndoredo::testMorphPreservesConnections()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(andGate);
    scene.addItem(led);

    auto *conn = new QNEConnection();
    conn->setStartPort(andGate->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    scene.addItem(conn);
    const int andId = andGate->id();

    // Morph AND → OR; andGate pointer becomes invalid after push
    scene.undoStack()->push(new MorphCommand({andGate}, ElementType::Or, &scene));

    // The OR element (with same ID) should have its output connected to LED
    auto *morphed = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    QVERIFY(morphed != nullptr);
    QCOMPARE(morphed->elementType(), ElementType::Or);
    QVERIFY(!morphed->outputPort(0)->connections().isEmpty());
    QCOMPARE(morphed->outputPort(0)->connections().constFirst()->endPort()->graphicElement()->id(), led->id());

    // Undo → AND gate with connection restored
    scene.undoStack()->undo();
    auto *restored = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->elementType(), ElementType::And);
    QVERIFY(!restored->outputPort(0)->connections().isEmpty());
}

// ─── ChangeInputSizeCommand ───────────────────────────────────────────────

void TestSceneUndoredo::testIncreaseInputSize()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    const int id = andGate->id();
    QCOMPARE(andGate->inputSize(), 2);

    scene.undoStack()->push(new ChangeInputSizeCommand({andGate}, 3, &scene));

    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->inputSize(), 3);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->inputSize(), 2);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->inputSize(), 3);
}

void TestSceneUndoredo::testDecreaseInputSizeRemovesConnection()
{
    Scene scene;

    // AND gate minInputSize=2, so we increase to 3 first, connect at port 2, then reduce to 2
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(andGate);
    scene.addItem(sw);
    const int andId = andGate->id();

    // Increase to 3 inputs directly (no command needed for setup)
    andGate->setInputSize(3);
    QCOMPARE(andGate->inputSize(), 3);

    // Connect switch output to AND's third input (index 2)
    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(andGate->inputPort(2));
    scene.addItem(conn);
    QVERIFY(!andGate->inputPort(2)->connections().isEmpty());

    // Reduce from 3 to 2 inputs via command → removes connection from port 2
    scene.undoStack()->push(new ChangeInputSizeCommand({andGate}, 2, &scene));

    auto *elm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    QCOMPARE(elm->inputSize(), 2);
    // Port 2 no longer exists after reduction

    // Undo → inputSize restored to 3, connection restored on port 2
    scene.undoStack()->undo();
    elm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    QCOMPARE(elm->inputSize(), 3);
    QVERIFY(!elm->inputPort(2)->connections().isEmpty());

    // Redo → reduce to 2 again
    scene.undoStack()->redo();
    elm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    QCOMPARE(elm->inputSize(), 2);
}

void TestSceneUndoredo::testDecreaseInputSizeMultipleConnections()
{
    Scene scene;

    // AND gate with 4 inputs, switches connected to ports 2 and 3
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw2 = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(andGate);
    scene.addItem(sw1);
    scene.addItem(sw2);
    const int andId = andGate->id();

    andGate->setInputSize(4);
    QCOMPARE(andGate->inputSize(), 4);

    auto *conn1 = new QNEConnection();
    conn1->setStartPort(sw1->outputPort(0));
    conn1->setEndPort(andGate->inputPort(2));
    scene.addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(sw2->outputPort(0));
    conn2->setEndPort(andGate->inputPort(3));
    scene.addItem(conn2);

    QVERIFY(!andGate->inputPort(2)->connections().isEmpty());
    QVERIFY(!andGate->inputPort(3)->connections().isEmpty());

    // Reduce from 4 to 2 — removes both connections
    scene.undoStack()->push(new ChangeInputSizeCommand({andGate}, 2, &scene));

    auto *elm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    QCOMPARE(elm->inputSize(), 2);

    // Undo — both connections restored
    scene.undoStack()->undo();
    elm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    QCOMPARE(elm->inputSize(), 4);
    QVERIFY(!elm->inputPort(2)->connections().isEmpty());
    QVERIFY(!elm->inputPort(3)->connections().isEmpty());

    // Redo — back to 2
    scene.undoStack()->redo();
    elm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    QCOMPARE(elm->inputSize(), 2);
}

// ─── ChangeOutputSizeCommand ──────────────────────────────────────────────

void TestSceneUndoredo::testChangeOutputSizeUndoRedo()
{
    Scene scene;

    // Demux: minOutput=2, maxOutput=8; default outputSize=2
    auto *demux = ElementFactory::buildElement(ElementType::Demux);
    scene.addItem(demux);
    const int id = demux->id();
    const int initialSize = demux->outputSize();
    QVERIFY(initialSize >= 2);

    // Manually set output size to 4 so we have room to decrease
    demux->setOutputSize(4);
    QCOMPARE(demux->outputSize(), 4);

    // Reduce from 4 to 2 via command
    scene.undoStack()->push(new ChangeOutputSizeCommand({demux}, 2, &scene));

    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->outputSize(), 2);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->outputSize(), 4);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->outputSize(), 2);
}

void TestSceneUndoredo::testChangeOutputSizeMultipleElements()
{
    Scene scene;

    // Two demuxes: one with a connection on port 2, one without
    auto *demux1 = ElementFactory::buildElement(ElementType::Demux);
    auto *demux2 = ElementFactory::buildElement(ElementType::Demux);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(demux1);
    scene.addItem(demux2);
    scene.addItem(led);
    const int id1 = demux1->id();
    const int id2 = demux2->id();

    demux1->setOutputSize(4);
    demux2->setOutputSize(4);

    // Connect only demux1's port 2 to LED
    auto *conn = new QNEConnection();
    conn->setStartPort(demux1->outputPort(2));
    conn->setEndPort(led->inputPort(0));
    scene.addItem(conn);

    QVERIFY(!demux1->outputPort(2)->connections().isEmpty());
    QVERIFY(demux2->outputPort(2)->connections().isEmpty());

    // Reduce both from 4 to 2 — demux1 loses 1 connection, demux2 loses 0
    scene.undoStack()->push(new ChangeOutputSizeCommand({demux1, demux2}, 2, &scene));

    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->outputSize(), 2);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->outputSize(), 2);

    // Undo — both restored to 4, demux1's connection restored
    scene.undoStack()->undo();
    auto *elm1 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1));
    auto *elm2 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2));
    QCOMPARE(elm1->outputSize(), 4);
    QCOMPARE(elm2->outputSize(), 4);
    QVERIFY(!elm1->outputPort(2)->connections().isEmpty());

    // Redo — back to 2
    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1))->outputSize(), 2);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2))->outputSize(), 2);
}

// ─── SplitCommand ─────────────────────────────────────────────────────────

void TestSceneUndoredo::testSplitConnectionCreatesNode()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    andGate->setPos(0, 0);
    led->setPos(200, 0);
    scene.addItem(andGate);
    scene.addItem(led);

    auto *conn = new QNEConnection();
    conn->setStartPort(andGate->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    scene.addItem(conn);

    QCOMPARE(scene.elements().size(), 2);

    // Split connection at midpoint → inserts a Node
    const QPointF midPoint(100, 0);
    scene.undoStack()->push(new SplitCommand(conn, midPoint, &scene));

    // After split: AND, LED, Node = 3 elements
    QCOMPARE(scene.elements().size(), 3);

    // Undo → back to AND + LED, original connection restored
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 2);
    // Original conn endpoints should be restored (AND output → LED input)
    QVERIFY(!andGate->outputPort(0)->connections().isEmpty());
    QVERIFY(!led->inputPort(0)->connections().isEmpty());

    // Redo → Node re-inserted
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 3);
}

// ─── UpdateCommand ────────────────────────────────────────────────────────

void TestSceneUndoredo::testUpdateCommandUndoRedo()
{
    Scene scene;

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    sw->setLabel("oldLabel");
    scene.addItem(sw);
    const int id = sw->id();

    // Capture old state BEFORE making changes
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        sw->save(stream);
    }

    // Change the label
    sw->setLabel("newLabel");

    // UpdateCommand captures current (new) state in constructor
    scene.undoStack()->push(new UpdateCommand({sw}, oldData, &scene));

    // redo() called on push → loads new state (newLabel)
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->label(), QString("newLabel"));

    // undo() → loads old state (oldLabel)
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->label(), QString("oldLabel"));

    // redo() → loads new state (newLabel)
    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->label(), QString("newLabel"));
}

// ─── Edge cases ───────────────────────────────────────────────────────────

void TestSceneUndoredo::testNewOperationClearsRedoStack()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    andGate->setSelected(true);
    scene.copyAction();

    // Build up undo history: 2 pastes
    scene.pasteAction();
    scene.pasteAction();
    QCOMPARE(scene.elements().size(), 3);

    // Undo one → redo stack has 1 item
    scene.undoStack()->undo();
    QVERIFY(scene.undoStack()->canRedo());

    // Perform new operation → redo stack must be cleared
    scene.pasteAction();
    QVERIFY(!scene.undoStack()->canRedo());
}

void TestSceneUndoredo::testUndoBeyondBeginning()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    andGate->setSelected(true);
    scene.copyAction();
    scene.pasteAction(); // 1 command on stack

    // Undo more times than commands — must not crash
    for (int i = 0; i < 10; ++i) {
        scene.undoStack()->undo();
    }

    // Stack is at the beginning — canUndo() is false
    QVERIFY(!scene.undoStack()->canUndo());
}

void TestSceneUndoredo::testRedoBeyondEnd()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    andGate->setSelected(true);
    scene.copyAction();
    scene.pasteAction(); // 1 command

    scene.undoStack()->undo(); // now at beginning, redo available

    // Redo more times than available — must not crash
    for (int i = 0; i < 10; ++i) {
        scene.undoStack()->redo();
    }

    // Stack is at the end — canRedo() is false
    QVERIFY(!scene.undoStack()->canRedo());
}

// ─── Additional FlipCommand edge cases ───────────────────────────────────

void TestSceneUndoredo::testFlipRotationChanges()
{
    // Documents the FIXME in Commands.cpp:644 — both axis=0 and axis=1 add +180°
    // to rotation for rotatable elements (AND gate is rotatable)
    Scene scene;

    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(96, 0);
    scene.addItem(elm1);
    scene.addItem(elm2);
    const int id1 = elm1->id();
    const int id2 = elm2->id();

    auto getRotation = [&](int id) {
        return dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))->rotation();
    };

    QCOMPARE(getRotation(id1), 0.0);
    QCOMPARE(getRotation(id2), 0.0);

    // Horizontal flip (axis=0): positions swap AND rotation += 180 (rotatable elements)
    scene.undoStack()->push(new FlipCommand({elm1, elm2}, 0, &scene));
    QCOMPARE(getRotation(id1), 180.0);
    QCOMPARE(getRotation(id2), 180.0);

    // Undo calls redo() again → rotation += 180 more = 360; GraphicElement normalizes 360 → 0
    scene.undoStack()->undo();
    QCOMPARE(getRotation(id1), 0.0);   // fmod(360, 360) == 0
    QCOMPARE(getRotation(id2), 0.0);

    // Now test vertical flip (axis=1) — FIXME: also adds +180° (same code path as axis=0)
    scene.undoStack()->push(new FlipCommand({elm1, elm2}, 1, &scene));
    QCOMPARE(getRotation(id1), 180.0); // 0 + 180
    QCOMPARE(getRotation(id2), 180.0);
}

void TestSceneUndoredo::testFlipNonRotatableElement()
{
    // InputSwitch is non-rotatable: position flips but rotation stays 0
    Scene scene;

    auto *sw1 = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw2 = ElementFactory::buildElement(ElementType::InputSwitch);
    sw1->setPos(0, 0);
    sw2->setPos(96, 0);
    scene.addItem(sw1);
    scene.addItem(sw2);
    const int id1 = sw1->id();
    const int id2 = sw2->id();

    QVERIFY(!sw1->isRotatable());
    QCOMPARE(sw1->rotation(), 0.0);
    QCOMPARE(sw2->rotation(), 0.0);

    const double x1 = sw1->pos().x(); // 0
    const double x2 = sw2->pos().x(); // 96

    // Horizontal flip: positions swap
    scene.undoStack()->push(new FlipCommand({sw1, sw2}, 0, &scene));

    auto *e1 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1));
    auto *e2 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2));
    QCOMPARE(e1->pos().x(), x2);
    QCOMPARE(e2->pos().x(), x1);

    // Rotation unchanged — non-rotatable elements are not affected
    QCOMPARE(e1->rotation(), 0.0);
    QCOMPARE(e2->rotation(), 0.0);

    // undo() calls redo() → positions flip back
    scene.undoStack()->undo();
    QCOMPARE(e1->pos().x(), x1);
    QCOMPARE(e2->pos().x(), x2);
    QCOMPARE(e1->rotation(), 0.0);
    QCOMPARE(e2->rotation(), 0.0);
}

void TestSceneUndoredo::testFlipSingleElementPositionUnchanged()
{
    // Single rotatable element: minX==maxX → position formula = identity, but rotation still += 180
    Scene scene;

    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(48, 48);
    scene.addItem(elm);
    const int id = elm->id();
    const QPointF originalPos = elm->pos();

    QVERIFY(elm->isRotatable());
    QCOMPARE(elm->rotation(), 0.0);

    // Push flip (axis=0, single element): minX = maxX = pos.x()
    // Formula: x_new = minX + (maxX - x) = x + (x - x) = x  → position unchanged
    scene.undoStack()->push(new FlipCommand({elm}, 0, &scene));

    auto *e = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
    QCOMPARE(e->pos(), originalPos);    // position unchanged (degenerate case)
    QCOMPARE(e->rotation(), 180.0);    // but rotation changes

    // undo() calls redo() → position still unchanged, rotation += 180 again = 360
    // GraphicElement::setRotation normalizes via fmod(360, 360) → 0
    scene.undoStack()->undo();
    QCOMPARE(e->pos(), originalPos);
    QCOMPARE(e->rotation(), 0.0);     // 360 normalized to 0 by GraphicElement::setRotation
}

// ─── Additional RotateCommand edge cases ─────────────────────────────────

void TestSceneUndoredo::testRotateMultipleElementsPositionsChange()
{
    // Two elements at different positions: rotation uses collective center as pivot
    // → both positions change; undo restores both to originals
    Scene scene;

    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(96, 0); // 96 = 12 * 8, grid-aligned
    scene.addItem(elm1);
    scene.addItem(elm2);
    const int id1 = elm1->id();
    const int id2 = elm2->id();
    const QPointF origPos1 = elm1->pos();
    const QPointF origPos2 = elm2->pos();

    scene.undoStack()->push(new RotateCommand({elm1, elm2}, 90, &scene));

    auto *e1 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id1));
    auto *e2 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id2));

    // Positions must change (both elements move away from their original positions)
    QVERIFY(e1->pos() != origPos1);
    QVERIFY(e2->pos() != origPos2);

    // Rotations must change
    QCOMPARE(e1->rotation(), 90.0);
    QCOMPARE(e2->rotation(), 90.0);

    // Undo → both positions and rotations restored
    scene.undoStack()->undo();
    QCOMPARE(e1->pos(), origPos1);
    QCOMPARE(e2->pos(), origPos2);
    QCOMPARE(e1->rotation(), 0.0);
    QCOMPARE(e2->rotation(), 0.0);

    // Redo → positions change again
    scene.undoStack()->redo();
    QVERIFY(e1->pos() != origPos1);
    QVERIFY(e2->pos() != origPos2);
}

// ─── Additional DeleteItemsCommand edge cases ─────────────────────────────

void TestSceneUndoredo::testDeleteMultipleElements()
{
    Scene scene;

    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *led  = ElementFactory::buildElement(ElementType::Led);
    and1->setPos(0, 0);
    and2->setPos(96, 0);
    led->setPos(192, 0);
    scene.addItem(and1);
    scene.addItem(and2);
    scene.addItem(led);
    QCOMPARE(scene.elements().size(), 3);

    // Select and delete all three simultaneously
    and1->setSelected(true);
    and2->setSelected(true);
    led->setSelected(true);
    scene.deleteAction();
    QCOMPARE(scene.elements().size(), 0);

    // Undo → all three restored
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 3);

    // Redo → all three gone again
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 0);
}

// ─── AddItemsCommand (direct) ─────────────────────────────────────────────

void TestSceneUndoredo::testAddItemsCommandDirect()
{
    // AddItemsCommand adds items in its constructor; first redo() is a no-op
    // because m_itemData is empty (items already on scene from construction)
    Scene scene;
    QCOMPARE(scene.elements().size(), 0);

    auto *elm = ElementFactory::buildElement(ElementType::And);
    // elm is NOT yet in scene

    // Constructor adds elm to scene immediately; push calls redo() but m_itemData is empty → no-op
    scene.undoStack()->push(new AddItemsCommand({elm}, &scene));
    QCOMPARE(scene.elements().size(), 1);

    // undo() serializes+deletes
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 0);

    // redo() deserializes+re-adds
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 1);

    // Second undo/redo cycle works identically
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 0);
    scene.undoStack()->redo();
    QCOMPARE(scene.elements().size(), 1);
}

// ─── Additional MorphCommand edge cases ───────────────────────────────────

void TestSceneUndoredo::testMorphMultipleElements()
{
    // Morph two elements of different types simultaneously to the same new type
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *orGate  = ElementFactory::buildElement(ElementType::Or);
    andGate->setPos(0, 0);
    orGate->setPos(96, 0);
    scene.addItem(andGate);
    scene.addItem(orGate);
    const int andId = andGate->id();
    const int orId  = orGate->id();

    // MorphCommand::undo() rebuilds original types from m_types = {And, Or}
    scene.undoStack()->push(new MorphCommand({andGate, orGate}, ElementType::Xor, &scene));

    // Both elements now Xor (same IDs preserved)
    auto *m1 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    auto *m2 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(orId));
    QVERIFY(m1 != nullptr);
    QVERIFY(m2 != nullptr);
    QCOMPARE(m1->elementType(), ElementType::Xor);
    QCOMPARE(m2->elementType(), ElementType::Xor);

    // Undo → AND and OR restored respectively
    scene.undoStack()->undo();
    auto *r1 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId));
    auto *r2 = dynamic_cast<GraphicElement *>(ElementFactory::itemById(orId));
    QVERIFY(r1 != nullptr);
    QVERIFY(r2 != nullptr);
    QCOMPARE(r1->elementType(), ElementType::And);
    QCOMPARE(r2->elementType(), ElementType::Or);

    // Redo → both Xor again
    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(andId))->elementType(), ElementType::Xor);
    QCOMPARE(dynamic_cast<GraphicElement *>(ElementFactory::itemById(orId))->elementType(), ElementType::Xor);
}

// ─── QUndoStack clean state ───────────────────────────────────────────────

void TestSceneUndoredo::testUndoStackCleanState()
{
    Scene scene;

    // Fresh scene: stack is clean (no operations)
    QVERIFY(scene.undoStack()->isClean());

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    andGate->setSelected(true);
    scene.copyAction();

    // After first operation: dirty
    scene.pasteAction();
    QVERIFY(!scene.undoStack()->isClean());

    // Undo back to beginning: clean again
    scene.undoStack()->undo();
    QVERIFY(scene.undoStack()->isClean());

    // Redo: dirty again
    scene.undoStack()->redo();
    QVERIFY(!scene.undoStack()->isClean());

    // setClean() marks the CURRENT position as the clean state
    scene.undoStack()->setClean();
    QVERIFY(scene.undoStack()->isClean());

    // A new operation after setClean() → dirty relative to clean marker
    scene.pasteAction();
    QVERIFY(!scene.undoStack()->isClean());

    // Undo back to clean marker position → clean again
    scene.undoStack()->undo();
    QVERIFY(scene.undoStack()->isClean());
}

void TestSceneUndoredo::testContextDirectoryPerTab()
{
    // Regression test for the autosave context-directory leak.
    // Before the fix, Workspace::autosave() set GlobalProperties::currentDir to the
    // autosave temp directory and never restored it. This caused IC relative-path
    // resolution to fail in other open workspaces after the first autosave.
    //
    // The fix: save and restore GlobalProperties::currentDir around the autosave write.

    const QString savedForTest = GlobalProperties::currentDir;

    // Simulate a "currently active workspace" context directory
    const QString circuitDir = m_tempDir.path() + "/my_project";
    QDir().mkpath(circuitDir);
    GlobalProperties::currentDir = circuitDir;

    {
        WorkSpace ws;
        // Push a command to dirty the undo stack, which triggers the autosave
        // signal chain: indexChanged → checkUpdateRequest → circuitHasChanged → autosave()
        auto *elm = ElementFactory::buildElement(ElementType::And);
        ws.scene()->receiveCommand(new AddItemsCommand({elm}, ws.scene()));
        // The signal chain is synchronous (direct connections, same thread),
        // so autosave() has already run by this point.
    }

    // currentDir must be restored to what it was before the WorkSpace did its autosave
    QCOMPARE(GlobalProperties::currentDir, circuitDir);

    GlobalProperties::currentDir = savedForTest;
}

