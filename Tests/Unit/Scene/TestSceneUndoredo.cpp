// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestSceneUndoredo.h"

#include <QDataStream>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoCommand>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Common/TestUtils.h"

void TestSceneUndoredo::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
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
    QVERIFY(scene.itemById(elmId) != nullptr);

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

    auto *after = dynamic_cast<GraphicElement *>(scene.itemById(elmId));
    QCOMPARE(after->rotation(), 90.0);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->rotation(), 0.0);
}

void TestSceneUndoredo::testRotateUndoRedo()
{
    Scene scene;

    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elm);
    const int elmId = elm->id();

    scene.undoStack()->push(new RotateCommand({elm}, 90, &scene));
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->rotation(), 90.0);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->rotation(), 0.0);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->rotation(), 90.0);
}

void TestSceneUndoredo::testRotateAccumulates()
{
    Scene scene;

    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elm);
    const int elmId = elm->id();

    // Two 90° rotations = 180° total
    scene.undoStack()->push(new RotateCommand({elm}, 90, &scene));
    elm = dynamic_cast<GraphicElement *>(scene.itemById(elmId));
    scene.undoStack()->push(new RotateCommand({elm}, 90, &scene));
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->rotation(), 180.0);

    // Undo second → 90°
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->rotation(), 90.0);

    // Undo first → 0°
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->rotation(), 0.0);
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

    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->pos(), newPos);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->pos(), oldPos);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(elmId))->pos(), newPos);
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

    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->pos(), new1);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->pos(), new2);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->pos(), old1);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->pos(), old2);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->pos(), new1);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->pos(), new2);
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

    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->pos().x(), x2);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->pos().x(), x1);

    // undo() calls redo() — flip is involution for positions
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->pos().x(), x1);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->pos().x(), x2);

    // redo() → flip again
    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->pos().x(), x2);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->pos().x(), x1);
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

    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->pos().y(), y2);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->pos().y(), y1);

    // undo() calls redo() — flip back
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->pos().y(), y1);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->pos().y(), y2);
}

// ─── MorphCommand ─────────────────────────────────────────────────────────

void TestSceneUndoredo::testMorphTypeChange()
{
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);
    const int id = andGate->id();

    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->elementType(), ElementType::And);

    // Push MorphCommand AND → OR; redo() is called immediately, andGate pointer becomes invalid
    scene.undoStack()->push(new MorphCommand({andGate}, ElementType::Or, &scene));

    auto *morphed = dynamic_cast<GraphicElement *>(scene.itemById(id));
    QVERIFY(morphed != nullptr);
    QCOMPARE(morphed->elementType(), ElementType::Or);

    // Undo → back to AND
    scene.undoStack()->undo();
    auto *restored = dynamic_cast<GraphicElement *>(scene.itemById(id));
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->elementType(), ElementType::And);

    // Redo → OR again
    scene.undoStack()->redo();
    auto *redone = dynamic_cast<GraphicElement *>(scene.itemById(id));
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

    auto *morphed = dynamic_cast<GraphicElement *>(scene.itemById(id));
    QVERIFY(morphed != nullptr);
    QCOMPARE(morphed->elementType(), ElementType::InputButton);
    QCOMPARE(morphed->label(), QString("testLabel"));

    // Undo → InputSwitch with label preserved
    scene.undoStack()->undo();
    auto *restored = dynamic_cast<GraphicElement *>(scene.itemById(id));
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
    auto *morphed = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    QVERIFY(morphed != nullptr);
    QCOMPARE(morphed->elementType(), ElementType::Or);
    QVERIFY(!morphed->outputPort(0)->connections().isEmpty());
    QCOMPARE(morphed->outputPort(0)->connections().constFirst()->endPort()->graphicElement()->id(), led->id());

    // Undo → AND gate with connection restored
    scene.undoStack()->undo();
    auto *restored = dynamic_cast<GraphicElement *>(scene.itemById(andId));
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

    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->inputSize(), 3);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->inputSize(), 2);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->inputSize(), 3);
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

    auto *elm = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    QCOMPARE(elm->inputSize(), 2);
    // Port 2 no longer exists after reduction

    // Undo → inputSize restored to 3, connection restored on port 2
    scene.undoStack()->undo();
    elm = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    QCOMPARE(elm->inputSize(), 3);
    QVERIFY(!elm->inputPort(2)->connections().isEmpty());

    // Redo → reduce to 2 again
    scene.undoStack()->redo();
    elm = dynamic_cast<GraphicElement *>(scene.itemById(andId));
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

    auto *elm = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    QCOMPARE(elm->inputSize(), 2);

    // Undo — both connections restored
    scene.undoStack()->undo();
    elm = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    QCOMPARE(elm->inputSize(), 4);
    QVERIFY(!elm->inputPort(2)->connections().isEmpty());
    QVERIFY(!elm->inputPort(3)->connections().isEmpty());

    // Redo — back to 2
    scene.undoStack()->redo();
    elm = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    QCOMPARE(elm->inputSize(), 2);
}

void TestSceneUndoredo::testDecreaseInputSizeRestoresConnectionWithOriginalId()
{
    // Connections severed by ChangeInputSizeCommand must be restored with their
    // original scene ID on undo, so that other undo commands (e.g.
    // DeleteItemsCommand) that stored that ID can still find them.
    Scene scene;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(andGate);
    scene.addItem(sw);
    const int andId = andGate->id();

    andGate->setInputSize(3);
    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(andGate->inputPort(2));
    scene.addItem(conn);
    const int originalConnId = conn->id();
    QVERIFY(originalConnId > 0);

    auto countSceneConnections = [&]() {
        int n = 0;
        for (auto *item : scene.items()) {
            if (qgraphicsitem_cast<QNEConnection *>(item)) { ++n; }
        }
        return n;
    };
    QCOMPARE(countSceneConnections(), 1);

    // Reduce to 2 inputs — connection on port 2 is severed
    scene.undoStack()->push(new ChangeInputSizeCommand({andGate}, 2, &scene));
    QVERIFY(scene.itemById(originalConnId) == nullptr);
    QCOMPARE(countSceneConnections(), 0);

    // Undo: connection must come back with its original ID
    scene.undoStack()->undo();
    auto *restoredConn = dynamic_cast<QNEConnection *>(scene.itemById(originalConnId));
    QVERIFY2(restoredConn != nullptr, "Connection must be findable by its original ID after undo");
    auto *elm = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    QVERIFY(!elm->inputPort(2)->connections().isEmpty());
    QCOMPARE(restoredConn->endPort()->graphicElement()->id(), andId);
    QCOMPARE(restoredConn->endPort()->index(), 2);
    QCOMPARE(countSceneConnections(), 1);

    // Redo: severed again, ID freed
    scene.undoStack()->redo();
    QVERIFY(scene.itemById(originalConnId) == nullptr);
    QCOMPARE(countSceneConnections(), 0);

    // Undo once more: ID stable across cycles
    scene.undoStack()->undo();
    QVERIFY2(scene.itemById(originalConnId) != nullptr, "ID must be stable across undo/redo cycles");
    QCOMPARE(countSceneConnections(), 1);
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

    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->outputSize(), 2);

    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->outputSize(), 4);

    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->outputSize(), 2);
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

    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->outputSize(), 2);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->outputSize(), 2);

    // Undo — both restored to 4, demux1's connection restored
    scene.undoStack()->undo();
    auto *elm1 = dynamic_cast<GraphicElement *>(scene.itemById(id1));
    auto *elm2 = dynamic_cast<GraphicElement *>(scene.itemById(id2));
    QCOMPARE(elm1->outputSize(), 4);
    QCOMPARE(elm2->outputSize(), 4);
    QVERIFY(!elm1->outputPort(2)->connections().isEmpty());

    // Redo — back to 2
    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id1))->outputSize(), 2);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id2))->outputSize(), 2);
}

void TestSceneUndoredo::testDecreaseOutputSizeRestoresConnectionWithOriginalId()
{
    // Same as the input-size ID test but for ChangeOutputSizeCommand.
    Scene scene;

    auto *demux = ElementFactory::buildElement(ElementType::Demux);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(demux);
    scene.addItem(led);
    const int demuxId = demux->id();

    demux->setOutputSize(4);
    auto *conn = new QNEConnection();
    conn->setStartPort(demux->outputPort(2));
    conn->setEndPort(led->inputPort(0));
    scene.addItem(conn);
    const int originalConnId = conn->id();
    QVERIFY(originalConnId > 0);

    auto countSceneConnections = [&]() {
        int n = 0;
        for (auto *item : scene.items()) {
            if (qgraphicsitem_cast<QNEConnection *>(item)) { ++n; }
        }
        return n;
    };
    QCOMPARE(countSceneConnections(), 1);

    // Reduce to 2 outputs — connection on port 2 is severed
    scene.undoStack()->push(new ChangeOutputSizeCommand({demux}, 2, &scene));
    QVERIFY(scene.itemById(originalConnId) == nullptr);
    QCOMPARE(countSceneConnections(), 0);

    // Undo: connection must come back with its original ID
    scene.undoStack()->undo();
    auto *restoredConn = dynamic_cast<QNEConnection *>(scene.itemById(originalConnId));
    QVERIFY2(restoredConn != nullptr, "Connection must be findable by its original ID after undo");
    auto *elm = dynamic_cast<GraphicElement *>(scene.itemById(demuxId));
    QVERIFY(!elm->outputPort(2)->connections().isEmpty());
    QCOMPARE(restoredConn->startPort()->graphicElement()->id(), demuxId);
    QCOMPARE(restoredConn->startPort()->index(), 2);
    QCOMPARE(countSceneConnections(), 1);

    // Redo: severed again
    scene.undoStack()->redo();
    QVERIFY(scene.itemById(originalConnId) == nullptr);
    QCOMPARE(countSceneConnections(), 0);

    // Undo once more: ID stable across cycles
    scene.undoStack()->undo();
    QVERIFY2(scene.itemById(originalConnId) != nullptr, "ID must be stable across undo/redo cycles");
    QCOMPARE(countSceneConnections(), 1);
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
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->label(), QString("newLabel"));

    // undo() → loads old state (oldLabel)
    scene.undoStack()->undo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->label(), QString("oldLabel"));

    // redo() → loads new state (newLabel)
    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(id))->label(), QString("newLabel"));
}

void TestSceneUndoredo::testUpdateCommandWirelessModeUndoRedo()
{
    // Changing wireless mode through UpdateCommand (the real ElementEditor path)
    // must be fully undoable and redoable.
    Scene scene;

    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    scene.addItem(nodeElm);
    const int id = nodeElm->id();

    auto *node = qobject_cast<Node *>(nodeElm);
    QVERIFY(node != nullptr);
    QCOMPARE(node->wirelessMode(), WirelessMode::None);

    // Capture old state (None mode) before making changes
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        node->save(stream);
    }

    // Change to Tx mode
    node->setWirelessMode(WirelessMode::Tx);

    // Push UpdateCommand — captures current (Tx) state as newData
    scene.undoStack()->push(new UpdateCommand({nodeElm}, oldData, &scene));

    // After push (redo): Tx mode
    auto *elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm != nullptr);
    QCOMPARE(elm->wirelessMode(), WirelessMode::Tx);

    // Undo → None mode
    scene.undoStack()->undo();
    elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm != nullptr);
    QCOMPARE(elm->wirelessMode(), WirelessMode::None);
    QVERIFY(elm->inputPort()->isRequired());

    // Redo → Tx mode again
    scene.undoStack()->redo();
    elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm != nullptr);
    QCOMPARE(elm->wirelessMode(), WirelessMode::Tx);
    QVERIFY(elm->inputPort()->isRequired());
}

void TestSceneUndoredo::testUpdateCommandRxModeIsRequired()
{
    // Undo of an Rx-mode change must restore isRequired=true on the input port.
    Scene scene;

    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    nodeElm->setLabel("CH");
    scene.addItem(nodeElm);
    const int id = nodeElm->id();

    auto *node = qobject_cast<Node *>(nodeElm);
    QVERIFY(node != nullptr);
    QVERIFY(node->inputPort()->isRequired());

    // Capture old state (None mode, required=true)
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        node->save(stream);
    }

    // Switch to Rx mode — input port becomes optional
    node->setWirelessMode(WirelessMode::Rx);
    QVERIFY(!node->inputPort()->isRequired());

    // Push UpdateCommand (captures Rx state)
    scene.undoStack()->push(new UpdateCommand({nodeElm}, oldData, &scene));

    // After push: Rx mode, port optional
    auto *elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm != nullptr);
    QCOMPARE(elm->wirelessMode(), WirelessMode::Rx);
    QVERIFY(!elm->inputPort()->isRequired());

    // Undo → None mode, port must be required again
    scene.undoStack()->undo();
    elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm != nullptr);
    QCOMPARE(elm->wirelessMode(), WirelessMode::None);
    QVERIFY(elm->inputPort()->isRequired());

    // Redo → Rx mode, port optional again
    scene.undoStack()->redo();
    elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm != nullptr);
    QCOMPARE(elm->wirelessMode(), WirelessMode::Rx);
    QVERIFY(!elm->inputPort()->isRequired());
}

void TestSceneUndoredo::testWirelessModeUndoRestoresConnection()
{
    // Setting a node to Tx mode should delete the output connection.
    // Undoing must restore the connection with its original ID.
    // This mirrors the macro command pattern used by ElementEditor::apply().
    Scene scene;

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(sw);
    scene.addItem(nodeElm);
    scene.addItem(led);

    const int nodeId = nodeElm->id();
    auto *node = qobject_cast<Node *>(nodeElm);
    QVERIFY(node != nullptr);

    // Wire: sw → node → led
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(sw->outputPort(0));
    conn1->setEndPort(node->inputPort());
    scene.addItem(conn1);
    const int conn1Id = conn1->id();

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(node->outputPort());
    conn2->setEndPort(led->inputPort(0));
    scene.addItem(conn2);
    const int conn2Id = conn2->id();

    // Snapshot old state (None mode)
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        node->save(stream);
    }

    // Set to Tx mode (hides output port) — connection not deleted by setWirelessMode
    node->setWirelessMode(WirelessMode::Tx);

    // Use the same macro pattern as ElementEditor::apply():
    // beginMacro → UpdateCommand → DeleteItemsCommand → endMacro
    scene.undoStack()->beginMacro(QStringLiteral("Change wireless mode"));
    scene.undoStack()->push(new UpdateCommand({nodeElm}, oldData, &scene));
    scene.undoStack()->push(new DeleteItemsCommand({static_cast<QGraphicsItem *>(conn2)}, &scene));
    scene.undoStack()->endMacro();

    // After macro: node is Tx, conn2 is deleted, conn1 still exists
    auto *n = dynamic_cast<Node *>(scene.itemById(nodeId));
    QVERIFY(n != nullptr);
    QCOMPARE(n->wirelessMode(), WirelessMode::Tx);
    QVERIFY(scene.itemById(conn1Id) != nullptr);
    QVERIFY(scene.itemById(conn2Id) == nullptr);

    // Undo macro: conn2 must be restored, node back to None
    scene.undoStack()->undo();
    n = dynamic_cast<Node *>(scene.itemById(nodeId));
    QVERIFY(n != nullptr);
    QCOMPARE(n->wirelessMode(), WirelessMode::None);
    QVERIFY(n->inputPort()->isRequired());

    auto *restoredConn2 = dynamic_cast<QNEConnection *>(scene.itemById(conn2Id));
    QVERIFY2(restoredConn2 != nullptr, "Output connection must be restored on undo");
    QCOMPARE(restoredConn2->startPort()->graphicElement()->id(), nodeId);
    QCOMPARE(restoredConn2->endPort()->graphicElement(), led);

    // conn1 must still exist
    QVERIFY(scene.itemById(conn1Id) != nullptr);

    // Redo: conn2 deleted again
    scene.undoStack()->redo();
    QVERIFY(scene.itemById(conn2Id) == nullptr);
    QCOMPARE(dynamic_cast<Node *>(scene.itemById(nodeId))->wirelessMode(), WirelessMode::Tx);

    // Undo again: stable across cycles
    scene.undoStack()->undo();
    QVERIFY2(scene.itemById(conn2Id) != nullptr, "Connection ID must be stable across undo/redo cycles");
}

void TestSceneUndoredo::testWirelessRxModeUndoRestoresConnection()
{
    // Setting a node to Rx mode should delete the input connection.
    // Undoing must restore it.
    Scene scene;

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(sw);
    scene.addItem(nodeElm);
    scene.addItem(led);

    const int nodeId = nodeElm->id();
    auto *node = qobject_cast<Node *>(nodeElm);
    QVERIFY(node != nullptr);

    // Wire: sw → node → led
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(sw->outputPort(0));
    conn1->setEndPort(node->inputPort());
    scene.addItem(conn1);
    const int conn1Id = conn1->id();

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(node->outputPort());
    conn2->setEndPort(led->inputPort(0));
    scene.addItem(conn2);
    const int conn2Id = conn2->id();

    // Snapshot old state
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        node->save(stream);
    }

    // Set to Rx mode (hides input port)
    node->setWirelessMode(WirelessMode::Rx);

    // Macro: UpdateCommand + DeleteItemsCommand for the input connection
    scene.undoStack()->beginMacro(QStringLiteral("Change wireless mode"));
    scene.undoStack()->push(new UpdateCommand({nodeElm}, oldData, &scene));
    scene.undoStack()->push(new DeleteItemsCommand({static_cast<QGraphicsItem *>(conn1)}, &scene));
    scene.undoStack()->endMacro();

    // After: conn1 deleted, conn2 exists
    QVERIFY(scene.itemById(conn1Id) == nullptr);
    QVERIFY(scene.itemById(conn2Id) != nullptr);
    QCOMPARE(dynamic_cast<Node *>(scene.itemById(nodeId))->wirelessMode(), WirelessMode::Rx);

    // Undo: conn1 restored
    scene.undoStack()->undo();
    auto *restoredConn1 = dynamic_cast<QNEConnection *>(scene.itemById(conn1Id));
    QVERIFY2(restoredConn1 != nullptr, "Input connection must be restored on undo");
    QCOMPARE(restoredConn1->endPort()->graphicElement()->id(), nodeId);
    QCOMPARE(dynamic_cast<Node *>(scene.itemById(nodeId))->wirelessMode(), WirelessMode::None);
}

void TestSceneUndoredo::testWirelessUndoRestoresPortVisibility()
{
    // Port visibility must be restored on undo/redo, not just mode and isRequired.
    Scene scene;

    auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
    scene.addItem(nodeElm);
    const int id = nodeElm->id();

    auto *node = qobject_cast<Node *>(nodeElm);
    QVERIFY(node != nullptr);

    // Capture old state (None mode — both ports visible)
    QVERIFY(node->inputPort()->isVisible());
    QVERIFY(node->outputPort()->isVisible());

    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        node->save(stream);
    }

    // Change to Tx mode — output port hidden
    node->setWirelessMode(WirelessMode::Tx);
    QVERIFY(node->inputPort()->isVisible());
    QVERIFY(!node->outputPort()->isVisible());

    scene.undoStack()->push(new UpdateCommand({nodeElm}, oldData, &scene));

    // After push (redo): Tx mode — output hidden
    auto *elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(!elm->outputPort()->isVisible());
    QVERIFY(elm->inputPort()->isVisible());

    // Undo → None — both visible
    scene.undoStack()->undo();
    elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm->inputPort()->isVisible());
    QVERIFY(elm->outputPort()->isVisible());

    // Redo → Tx — output hidden again
    scene.undoStack()->redo();
    elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm->inputPort()->isVisible());
    QVERIFY(!elm->outputPort()->isVisible());

    // Undo and change to Rx — input hidden, output visible
    scene.undoStack()->undo();
    elm = dynamic_cast<Node *>(scene.itemById(id));

    QByteArray noneData;
    {
        QDataStream stream(&noneData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        elm->save(stream);
    }

    elm->setWirelessMode(WirelessMode::Rx);
    QVERIFY(!elm->inputPort()->isVisible());
    QVERIFY(elm->outputPort()->isVisible());

    scene.undoStack()->push(new UpdateCommand({nodeElm}, noneData, &scene));

    // Undo Rx → None — both visible
    scene.undoStack()->undo();
    elm = dynamic_cast<Node *>(scene.itemById(id));
    QVERIFY(elm->inputPort()->isVisible());
    QVERIFY(elm->outputPort()->isVisible());
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
    // Flip toggles the mirror flag on rotatable elements (no rotation change)
    Scene scene;

    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(96, 0);
    scene.addItem(elm1);
    scene.addItem(elm2);
    const int id1 = elm1->id();
    const int id2 = elm2->id();

    auto getElm = [&](int id) {
        return dynamic_cast<GraphicElement *>(scene.itemById(id));
    };

    QVERIFY(!getElm(id1)->isFlippedX());
    QVERIFY(!getElm(id2)->isFlippedX());

    // Horizontal flip (axis=0): positions swap AND flippedX toggled
    scene.undoStack()->push(new FlipCommand({elm1, elm2}, 0, &scene));
    QVERIFY(getElm(id1)->isFlippedX());
    QVERIFY(getElm(id2)->isFlippedX());
    QCOMPARE(getElm(id1)->rotation(), 0.0);

    // Undo toggles flippedX back to false
    scene.undoStack()->undo();
    QVERIFY(!getElm(id1)->isFlippedX());
    QVERIFY(!getElm(id2)->isFlippedX());

    // Vertical flip (axis=1): mirrors Y positions AND flippedY toggled
    scene.undoStack()->push(new FlipCommand({elm1, elm2}, 1, &scene));
    QVERIFY(getElm(id1)->isFlippedY());
    QVERIFY(getElm(id2)->isFlippedY());
    QVERIFY(!getElm(id1)->isFlippedX());
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

    auto *e1 = dynamic_cast<GraphicElement *>(scene.itemById(id1));
    auto *e2 = dynamic_cast<GraphicElement *>(scene.itemById(id2));
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
    // Single rotatable element: minX==maxX → position formula = identity, but flip flag toggles
    Scene scene;

    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(48, 48);
    scene.addItem(elm);
    const int id = elm->id();
    const QPointF originalPos = elm->pos();

    QVERIFY(elm->isRotatable());
    QVERIFY(!elm->isFlippedX());

    // Push flip (axis=0, single element): minX = maxX = pos.x()
    // Formula: x_new = minX + (maxX - x) = x + (x - x) = x  → position unchanged
    scene.undoStack()->push(new FlipCommand({elm}, 0, &scene));

    auto *e = dynamic_cast<GraphicElement *>(scene.itemById(id));
    QCOMPARE(e->pos(), originalPos);    // position unchanged (degenerate case)
    QVERIFY(e->isFlippedX());           // but flip flag toggles

    // undo() calls redo() → position still unchanged, flippedX toggled back
    scene.undoStack()->undo();
    QCOMPARE(e->pos(), originalPos);
    QVERIFY(!e->isFlippedX());
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

    auto *e1 = dynamic_cast<GraphicElement *>(scene.itemById(id1));
    auto *e2 = dynamic_cast<GraphicElement *>(scene.itemById(id2));

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
    auto *m1 = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    auto *m2 = dynamic_cast<GraphicElement *>(scene.itemById(orId));
    QVERIFY(m1 != nullptr);
    QVERIFY(m2 != nullptr);
    QCOMPARE(m1->elementType(), ElementType::Xor);
    QCOMPARE(m2->elementType(), ElementType::Xor);

    // Undo → AND and OR restored respectively
    scene.undoStack()->undo();
    auto *r1 = dynamic_cast<GraphicElement *>(scene.itemById(andId));
    auto *r2 = dynamic_cast<GraphicElement *>(scene.itemById(orId));
    QVERIFY(r1 != nullptr);
    QVERIFY(r2 != nullptr);
    QCOMPARE(r1->elementType(), ElementType::And);
    QCOMPARE(r2->elementType(), ElementType::Or);

    // Redo → both Xor again
    scene.undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(andId))->elementType(), ElementType::Xor);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene.itemById(orId))->elementType(), ElementType::Xor);
}

void TestSceneUndoredo::testMorphDisplay14ToDisplay7RemovesDanglingConnection()
{
    // Regression: morphing Display14 (15 inputs) to Display7 (8 inputs) must delete
    // any QNEConnection whose target port no longer exists on the new element, and
    // undo must restore that connection on the recreated Display14.
    Scene scene;

    auto *disp14 = new Display14();
    auto *sw     = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(disp14);
    scene.addItem(sw);
    const int dispId = disp14->id();
    const int swId   = sw->id();

    // Display14 has 15 inputs (ports 0-14); connect to the last one
    QCOMPARE(disp14->inputSize(), 15);
    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(disp14->inputPort(14));
    scene.addItem(conn);

    QVERIFY(!disp14->inputPort(14)->connections().isEmpty());

    auto countSceneConnections = [&]() {
        int n = 0;
        for (auto *item : scene.items()) {
            if (qgraphicsitem_cast<QNEConnection *>(item)) { ++n; }
        }
        return n;
    };
    QCOMPARE(countSceneConnections(), 1);

    // ── redo ──────────────────────────────────────────────────────────────
    scene.undoStack()->push(new MorphCommand({disp14}, ElementType::Display7, &scene));

    auto *disp7 = dynamic_cast<GraphicElement *>(scene.itemById(dispId));
    QVERIFY(disp7 != nullptr);
    QCOMPARE(disp7->elementType(), ElementType::Display7);
    QCOMPARE(disp7->inputSize(), 8);
    QCOMPARE(countSceneConnections(), 0);
    QVERIFY(sw->outputPort(0)->connections().isEmpty());

    // ── undo: Display14 restored, wire on port 14 must come back ──────────
    scene.undoStack()->undo();

    auto *restored14 = dynamic_cast<GraphicElement *>(scene.itemById(dispId));
    QVERIFY(restored14 != nullptr);
    QCOMPARE(restored14->elementType(), ElementType::Display14);
    QCOMPARE(restored14->inputSize(), 15);
    QCOMPARE(countSceneConnections(), 1);
    QVERIFY(!restored14->inputPort(14)->connections().isEmpty());
    QCOMPARE(restored14->inputPort(14)->connections().constFirst()->startPort()->graphicElement()->id(), swId);

    // ── redo again: connection deleted once more, no dangling wire ─────────
    scene.undoStack()->redo();

    auto *redisp7 = dynamic_cast<GraphicElement *>(scene.itemById(dispId));
    QVERIFY(redisp7 != nullptr);
    QCOMPARE(redisp7->elementType(), ElementType::Display7);
    QCOMPARE(countSceneConnections(), 0);
    for (auto *item : scene.items()) {
        if (auto *c = qgraphicsitem_cast<QNEConnection *>(item)) {
            QVERIFY(c->startPort() != nullptr);
            QVERIFY(c->endPort()   != nullptr);
        }
    }
}

void TestSceneUndoredo::testMorphToFewerPortsAllDroppedConnectionsRemoved()
{
    // When morphing to an element with fewer ports, ALL connections on removed
    // ports must be deleted, while connections on preserved ports are kept.
    Scene scene;

    auto *disp14 = new Display14();
    scene.addItem(disp14);
    const int dispId = disp14->id();
    QCOMPARE(disp14->inputSize(), 15);

    // Wire up three switches to three different ports:
    //   sw0 → port 0  (preserved: Display7 also has port 0)
    //   sw7 → port 7  (preserved: Display7 has ports 0-7)
    //   sw14 → port 14 (dropped: Display7 only has ports 0-7)
    auto *sw0  = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw7  = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *sw14 = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(sw0);
    scene.addItem(sw7);
    scene.addItem(sw14);

    auto *c0 = new QNEConnection();
    c0->setStartPort(sw0->outputPort(0));
    c0->setEndPort(disp14->inputPort(0));
    scene.addItem(c0);

    auto *c7 = new QNEConnection();
    c7->setStartPort(sw7->outputPort(0));
    c7->setEndPort(disp14->inputPort(7));
    scene.addItem(c7);

    auto *c14 = new QNEConnection();
    c14->setStartPort(sw14->outputPort(0));
    c14->setEndPort(disp14->inputPort(14));
    scene.addItem(c14);

    auto countSceneConnections = [&]() {
        int n = 0;
        for (auto *item : scene.items()) {
            if (qgraphicsitem_cast<QNEConnection *>(item)) {
                ++n;
            }
        }
        return n;
    };
    QCOMPARE(countSceneConnections(), 3);

    const int sw0Id  = sw0->id();
    const int sw7Id  = sw7->id();
    const int sw14Id = sw14->id();

    // ── redo ──────────────────────────────────────────────────────────────
    scene.undoStack()->push(new MorphCommand({disp14}, ElementType::Display7, &scene));

    auto *disp7 = dynamic_cast<GraphicElement *>(scene.itemById(dispId));
    QVERIFY(disp7 != nullptr);
    QCOMPARE(disp7->elementType(), ElementType::Display7);
    QCOMPARE(disp7->inputSize(), 8);

    // Connections on ports 0 and 7 must survive; connection on port 14 must be gone
    QCOMPARE(countSceneConnections(), 2);
    QVERIFY(!disp7->inputPort(0)->connections().isEmpty());
    QVERIFY(!disp7->inputPort(7)->connections().isEmpty());
    QVERIFY(sw14->outputPort(0)->connections().isEmpty());
    for (auto *item : scene.items()) {
        if (auto *c = qgraphicsitem_cast<QNEConnection *>(item)) {
            QVERIFY(c->startPort() != nullptr);
            QVERIFY(c->endPort()   != nullptr);
        }
    }

    // ── undo: Display14 restored, all three connections must be back ───────
    scene.undoStack()->undo();

    auto *restored14 = dynamic_cast<GraphicElement *>(scene.itemById(dispId));
    QVERIFY(restored14 != nullptr);
    QCOMPARE(restored14->elementType(), ElementType::Display14);
    QCOMPARE(restored14->inputSize(), 15);
    QCOMPARE(countSceneConnections(), 3);
    QVERIFY(!restored14->inputPort(0)->connections().isEmpty());
    QVERIFY(!restored14->inputPort(7)->connections().isEmpty());
    QVERIFY(!restored14->inputPort(14)->connections().isEmpty());
    QCOMPARE(restored14->inputPort(0)->connections().constFirst()->startPort()->graphicElement()->id(),  sw0Id);
    QCOMPARE(restored14->inputPort(7)->connections().constFirst()->startPort()->graphicElement()->id(),  sw7Id);
    QCOMPARE(restored14->inputPort(14)->connections().constFirst()->startPort()->graphicElement()->id(), sw14Id);
}

void TestSceneUndoredo::testMorphUndoRestoresConnectionWithOriginalId()
{
    // After MorphCommand::undo() restores a deleted connection, the connection must
    // get back its original scene ID so that any undo command that stored that ID
    // (e.g. DeleteItemsCommand) can still find it via scene->itemById().
    Scene scene;

    auto *disp14 = new Display14();
    auto *sw     = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(disp14);
    scene.addItem(sw);
    const int dispId = disp14->id();

    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(disp14->inputPort(14));
    scene.addItem(conn);
    const int originalConnId = conn->id();
    QVERIFY(originalConnId > 0);

    // Morph Display14 → Display7: connection on port 14 is deleted
    scene.undoStack()->push(new MorphCommand({disp14}, ElementType::Display7, &scene));
    QVERIFY(scene.itemById(originalConnId) == nullptr);

    // Undo: connection must come back with its original ID
    scene.undoStack()->undo();

    auto *restored14 = dynamic_cast<GraphicElement *>(scene.itemById(dispId));
    QVERIFY(restored14 != nullptr);
    QCOMPARE(restored14->elementType(), ElementType::Display14);

    auto *restoredConn = dynamic_cast<QNEConnection *>(scene.itemById(originalConnId));
    QVERIFY2(restoredConn != nullptr, "Connection must be findable by its original ID after undo");
    QCOMPARE(restoredConn->endPort()->graphicElement()->id(), dispId);
    QCOMPARE(restoredConn->endPort()->index(), 14);

    // Redo: connection deleted again, same original ID slot freed
    scene.undoStack()->redo();
    QVERIFY(scene.itemById(originalConnId) == nullptr);

    // Undo once more: ID is stable across repeated undo/redo cycles
    scene.undoStack()->undo();
    QVERIFY2(scene.itemById(originalConnId) != nullptr, "ID must be stable across undo/redo cycles");
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

// ─── Multi-workspace cross-tab isolation ──────────────────────────────────

void TestSceneUndoredo::testCrossTabIdIsolation()
{
    // After the per-scene ID refactoring each WorkSpace owns an independent
    // Scene with its own ID counter starting at 1. Elements in different tabs
    // share the same numeric IDs but live in separate registries, so there is
    // no cross-tab ID collision or registry overwrite.

    WorkSpace wsA;
    WorkSpace wsB;

    auto *sceneA = wsA.scene();
    auto *sceneB = wsB.scene();

    // Add elements to workspace A — gets IDs 1..5 in sceneA
    QList<GraphicElement *> elemsA;
    for (int i = 0; i < 5; ++i) {
        auto *gate = ElementFactory::buildElement(ElementType::And);
        sceneA->addItem(gate);
        QVERIFY2(gate->id() > 0, "sceneA element should have positive ID");
        elemsA.append(gate);
    }

    // Add elements to workspace B — also gets IDs 1..5 in sceneB (independent counter)
    QList<GraphicElement *> elemsB;
    for (int i = 0; i < 5; ++i) {
        auto *gate = ElementFactory::buildElement(ElementType::And);
        sceneB->addItem(gate);
        QVERIFY2(gate->id() > 0, "sceneB element should have positive ID");
        elemsB.append(gate);
    }

    // IDs in both scenes start from 1 — by design (independent counters)
    QCOMPARE(elemsA.first()->id(), 1);
    QCOMPARE(elemsB.first()->id(), 1);

    // sceneA resolves its elements correctly
    for (auto *e : elemsA) {
        auto *found = dynamic_cast<GraphicElement *>(sceneA->itemById(e->id()));
        QVERIFY2(found == e, "sceneA must resolve each ID to its own element");
    }

    // sceneB resolves its elements correctly
    for (auto *e : elemsB) {
        auto *found = dynamic_cast<GraphicElement *>(sceneB->itemById(e->id()));
        QVERIFY2(found == e, "sceneB must resolve each ID to its own element");
    }

    // sceneA must not return sceneB's elements (registries are independent)
    for (auto *e : elemsB) {
        auto *inA = sceneA->itemById(e->id());
        QVERIFY2(inA != e, "sceneA registry must not contain sceneB's elements");
    }

    // sceneB must not return sceneA's elements
    for (auto *e : elemsA) {
        auto *inB = sceneB->itemById(e->id());
        QVERIFY2(inB != e, "sceneB registry must not contain sceneA's elements");
    }
}

void TestSceneUndoredo::testContextDirectoryPerTab()
{
    // Regression test: autosave must not alter the scene's contextDir.
    // The old global Serialization::contextDir was corrupted by autosave writes;
    // now that contextDir lives on Scene, autosave simply doesn't touch it.

    const QString circuitDir = m_tempDir.path() + "/my_project";
    QDir().mkpath(circuitDir);

    {
        WorkSpace ws;
        ws.scene()->setContextDir(circuitDir);
        // Push a command to dirty the undo stack, which triggers the autosave
        // signal chain: indexChanged → checkUpdateRequest → circuitHasChanged → autosave()
        auto *elm = ElementFactory::buildElement(ElementType::And);
        ws.scene()->receiveCommand(new AddItemsCommand({elm}, ws.scene()));
        // The signal chain is synchronous (direct connections, same thread),
        // so autosave() has already run by this point.

        // Scene's contextDir must not be changed by the autosave
        QCOMPARE(ws.scene()->contextDir(), circuitDir);
    }
}

// ─── Serialization::serialize portMap collision regression ────────────────
//
// Regression tests for the bug where Serialization::serialize() assigned
// sequential temp IDs starting from 1 to ALL elements, including those that
// already had valid positive scene IDs. In CommandUtils::saveItems(), the
// "other" elements (those that stay in the scene) are written to the same
// stream BEFORE serialize() is called, using their real IDs. When a deleted
// element received temp ID=1 it would collide with the "other" element whose
// real ID was also 1, corrupting the portMap on the subsequent load (undo).
// The fix: only assign temp IDs to elements with id <= 0; elements with
// valid positive IDs keep them unchanged.

void TestSceneUndoredo::testDeleteUndoConnectionsReattachedCorrectly()
{
    // Structural check: after delete + undo, each restored connection must
    // reference the correct GraphicElement at both endpoints.
    //
    // Bug scenario: swIn(ID=1) → AND(ID=2) → LED(ID=3).
    // DeleteItemsCommand removes AND; "others" are swIn and LED.
    // On undo, Serialization::serialize() used to give AND temp ID=1,
    // colliding with swIn's serialId. The portMap resolver then found
    // AND.in[0] instead of swIn.out[0] as the start-port of conn1.
    Scene scene;

    auto *swIn = new InputSwitch();
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = new Led();
    swIn->setPos(0, 0);
    andGate->setPos(96, 0);
    led->setPos(192, 0);
    scene.addItem(swIn);   // ID=1
    scene.addItem(andGate); // ID=2
    scene.addItem(led);    // ID=3

    auto *conn1 = new QNEConnection();
    conn1->setStartPort(swIn->outputPort(0));
    conn1->setEndPort(andGate->inputPort(0));
    scene.addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(andGate->outputPort(0));
    conn2->setEndPort(led->inputPort(0));
    scene.addItem(conn2);

    // Capture the IDs of the elements we expect to survive the undo
    const int swInId = swIn->id();
    const int ledId = led->id();

    // Delete AND gate and its two connections
    andGate->setSelected(true);
    scene.deleteAction();
    QCOMPARE(scene.elements().size(), 2);

    // Undo — AND and both connections must come back
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 3);

    // Retrieve elements by ID (pointers may have been recreated)
    auto *swInAfter  = dynamic_cast<GraphicElement *>(scene.itemById(swInId));
    auto *ledAfter   = dynamic_cast<GraphicElement *>(scene.itemById(ledId));
    QVERIFY2(swInAfter != nullptr, "swIn must still be in scene after undo");
    QVERIFY2(ledAfter  != nullptr, "LED must still be in scene after undo");

    // AND gate must have exactly one connection on each port
    // (find it via the switch's output connection)
    QVERIFY2(!swInAfter->outputPort(0)->connections().isEmpty(),
             "swIn.out[0] must be connected after undo");
    auto *restoredConn1 = swInAfter->outputPort(0)->connections().first();

    // The connection that starts at swIn.out[0] must end at AND.in[0] —
    // before the fix it ended at AND.in[0] using AND's portMap slot
    // overwriting swIn's, which self-looped the connection.
    auto *andEnd = restoredConn1->endPort();
    QVERIFY2(andEnd != nullptr, "conn1 must have a valid end port");
    auto *andAfter = andEnd->graphicElement();
    QVERIFY2(andAfter != nullptr, "conn1 end port must reference an element");
    QVERIFY2(andAfter != swInAfter, "conn1 must NOT self-loop back to swIn");
    QVERIFY2(andAfter != ledAfter,  "conn1 must NOT jump straight to LED");

    // The AND gate's output must lead to LED
    QVERIFY2(!andAfter->outputPort(0)->connections().isEmpty(),
             "AND.out[0] must be connected after undo");
    auto *restoredConn2 = andAfter->outputPort(0)->connections().first();
    QCOMPARE(restoredConn2->endPort()->graphicElement(), ledAfter);
}

void TestSceneUndoredo::testDeleteUndoRedoConnectionCountStable()
{
    // After any number of delete/undo/redo cycles, the total connection
    // count in the scene must return to its original value on every undo.
    Scene scene;

    auto *swA = new InputSwitch();
    auto *swB = new InputSwitch();
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = new Led();
    swA->setPos(0, 0);
    swB->setPos(0, 48);
    andGate->setPos(96, 0);
    led->setPos(192, 0);
    scene.addItem(swA);
    scene.addItem(swB);
    scene.addItem(andGate);
    scene.addItem(led);

    auto *c1 = new QNEConnection();
    c1->setStartPort(swA->outputPort(0));
    c1->setEndPort(andGate->inputPort(0));
    scene.addItem(c1);

    auto *c2 = new QNEConnection();
    c2->setStartPort(swB->outputPort(0));
    c2->setEndPort(andGate->inputPort(1));
    scene.addItem(c2);

    auto *c3 = new QNEConnection();
    c3->setStartPort(andGate->outputPort(0));
    c3->setEndPort(led->inputPort(0));
    scene.addItem(c3);

    // Helper: count live connections by scanning all element ports
    auto countConnections = [&]() {
        int n = 0;
        QSet<QNEConnection *> seen;
        for (auto *elm : scene.elements()) {
            for (auto *port : elm->outputs()) {
                for (auto *conn : port->connections()) {
                    if (!seen.contains(conn)) { seen.insert(conn); ++n; }
                }
            }
        }
        return n;
    };

    QCOMPARE(scene.elements().size(), 4);
    QCOMPARE(countConnections(), 3);

    const int andId = andGate->id();

    // Three delete/undo/redo cycles — connection count must be stable.
    // Note: after each undo the AND gate is re-created from serialized data,
    // so the original pointer is stale — always look it up by ID.
    for (int cycle = 0; cycle < 3; ++cycle) {
        auto *and_ = dynamic_cast<GraphicElement *>(scene.itemById(andId));
        QVERIFY2(and_ != nullptr, "AND gate must be in scene at start of cycle");

        and_->setSelected(true);
        scene.deleteAction();
        QCOMPARE(scene.elements().size(), 3); // swA, swB, LED remain
        QCOMPARE(countConnections(), 0);      // AND's 3 connections removed

        scene.undoStack()->undo();
        QCOMPARE(scene.elements().size(), 4);
        QCOMPARE(countConnections(), 3);

        scene.undoStack()->redo();
        QCOMPARE(scene.elements().size(), 3);
        QCOMPARE(countConnections(), 0);

        scene.undoStack()->undo();
        QCOMPARE(scene.elements().size(), 4);
        QCOMPARE(countConnections(), 3);
    }
}

void TestSceneUndoredo::testDeleteChainMiddleUndoRestoresTopology()
{
    // Three-element chain: sw → NOT → LED
    // Deleting NOT severs both connections; undo must restore both links
    // and the portMap must map each connection to the correct port objects.
    Scene scene;

    auto *sw  = new InputSwitch();
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    auto *led = new Led();
    sw->setPos(0, 0);
    notGate->setPos(96, 0);
    led->setPos(192, 0);
    scene.addItem(sw);
    scene.addItem(notGate);
    scene.addItem(led);

    const int swId  = sw->id();
    const int notId = notGate->id();
    const int ledId = led->id();

    auto *cIn = new QNEConnection();
    cIn->setStartPort(sw->outputPort(0));
    cIn->setEndPort(notGate->inputPort(0));
    scene.addItem(cIn);

    auto *cOut = new QNEConnection();
    cOut->setStartPort(notGate->outputPort(0));
    cOut->setEndPort(led->inputPort(0));
    scene.addItem(cOut);

    QCOMPARE(scene.elements().size(), 3);

    // Delete NOT (middle of chain)
    notGate->setSelected(true);
    scene.deleteAction();
    QCOMPARE(scene.elements().size(), 2); // sw + LED remain

    // Undo — NOT and both connections restored
    scene.undoStack()->undo();
    QCOMPARE(scene.elements().size(), 3);

    auto *swR   = dynamic_cast<GraphicElement *>(scene.itemById(swId));
    auto *notR  = dynamic_cast<GraphicElement *>(scene.itemById(notId));
    auto *ledR  = dynamic_cast<GraphicElement *>(scene.itemById(ledId));
    QVERIFY(swR  != nullptr);
    QVERIFY(notR != nullptr);
    QVERIFY(ledR != nullptr);

    // sw.out[0] → NOT.in[0]
    QVERIFY2(!swR->outputPort(0)->connections().isEmpty(),
             "sw.out[0] must be reconnected after undo");
    auto *restoredIn = swR->outputPort(0)->connections().first();
    QCOMPARE(restoredIn->endPort()->graphicElement(), notR);

    // NOT.out[0] → LED.in[0]
    QVERIFY2(!notR->outputPort(0)->connections().isEmpty(),
             "NOT.out[0] must be reconnected after undo");
    auto *restoredOut = notR->outputPort(0)->connections().first();
    QCOMPARE(restoredOut->endPort()->graphicElement(), ledR);

    // LED.in[0] must have exactly one connection (not duplicated or missing)
    QCOMPARE(ledR->inputPort(0)->connections().size(), 1);
}

void TestSceneUndoredo::testDeleteUndoSimulationCorrectness()
{
    // End-to-end simulation check: delete the AND gate that drives a LED,
    // undo the deletion, then verify the AND gate truth table through the
    // restored connections.  This is the exact scenario that was silently
    // broken by the portMap key collision in Serialization::serialize().
    WorkSpace ws;

    auto *swA     = new InputSwitch();
    auto *swB     = new InputSwitch();
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led     = new Led();
    swA->setPos(0,  0);
    swB->setPos(0, 48);
    andGate->setPos(96, 0);
    led->setPos(192, 0);

    CircuitBuilder builder(ws.scene());
    builder.add(swA, swB, andGate, led);
    builder.connect(swA, 0, andGate, 0);
    builder.connect(swB, 0, andGate, 1);
    builder.connect(andGate, 0, led, 0);
    builder.initSimulation();

    // Capture the AND gate's ID before deletion (pointer becomes invalid after undo)
    const int andId = andGate->id();

    // Delete AND gate
    andGate->setSelected(true);
    ws.scene()->deleteAction();
    QCOMPARE(ws.scene()->elements().size(), 3); // swA, swB, LED

    // Undo
    ws.scene()->undoStack()->undo();
    QCOMPARE(ws.scene()->elements().size(), 4);
    QVERIFY2(ws.scene()->itemById(andId) != nullptr, "AND gate must be back after undo");

    // Re-initialize simulation with the restored topology
    auto *sim = ws.simulation();
    sim->initialize();

    const QVector<QPair<QPair<bool, bool>, bool>> table = {
        {{false, false}, false},
        {{false, true},  false},
        {{true,  false}, false},
        {{true,  true},  true},
    };
    // Verify AND truth table through simulation
    for (const auto &row : table) {
        swA->setOn(row.first.first);
        swB->setOn(row.first.second);
        sim->update();
        QCOMPARE(TestUtils::getInputStatus(led), row.second);
    }
}

