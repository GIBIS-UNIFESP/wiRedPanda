// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Commands/TestCommands.h"

#include <QImage>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/InputButton.h"
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

namespace {
/// Test-only accessor: GraphicElement::pixmap() is protected, but a subclass may read it for its
/// own instances. This exposes the currently displayed pixmap so flip-variant rendering can be
/// asserted without widening the production API.
template <typename Element>
struct PixmapProbe : Element {
    QImage displayedImage() const { return this->pixmap().toImage(); }
};
} // namespace

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

void TestCommands::testFlipTextPixmapVariant()
{
    // Memory elements bake their pin letters (D, E, Q...) into the SVG. Flipping must swap in a
    // text-corrected pixmap variant (so the glyphs stay upright after the item-level mirror),
    // while a text-free element keeps its base pixmap (the item transform alone handles it).
    PixmapProbe<DLatch> latch;
    const QImage baseImage = latch.displayedImage();
    QVERIFY(!baseImage.isNull());

    latch.setFlippedX(true);
    QVERIFY2(latch.displayedImage() != baseImage,
             "Flipping a latch must produce a text-corrected pixmap variant");

    // Unflipping must round-trip back to exactly the base pixmap (zero regression).
    latch.setFlippedX(false);
    QCOMPARE(latch.displayedImage(), baseImage);

    // A text-free element re-uses its base pixmap when flipped (no redundant re-render).
    PixmapProbe<And> gate;
    const QImage gateBase = gate.displayedImage();
    gate.setFlippedX(true);
    QCOMPARE(gate.displayedImage(), gateBase);
}

void TestCommands::testRotateTextPixmapVariant()
{
    // Same idea as flipping, for rotation: a rotated memory element must swap in a pixmap whose
    // baked-in <text> is counter-rotated, so the pin letters stay upright while the body rotates.
    PixmapProbe<DLatch> latch;
    const QImage baseImage = latch.displayedImage();
    QVERIFY(!baseImage.isNull());

    latch.setRotation(90);
    QVERIFY2(latch.displayedImage() != baseImage,
             "Rotating a latch must produce a text-corrected pixmap variant");

    // Rotating back to 0 must round-trip to exactly the base pixmap (zero regression).
    latch.setRotation(0);
    QCOMPARE(latch.displayedImage(), baseImage);

    // A text-free element re-uses its base pixmap when rotated (no redundant re-render).
    PixmapProbe<And> gate;
    const QImage gateBase = gate.displayedImage();
    gate.setRotation(90);
    QCOMPARE(gate.displayedImage(), gateBase);
}

void TestCommands::testFlipRotateTextPixmapVariant()
{
    // The combined rotate+flip state is the one orientation case whose correction order matters
    // (the glyph carries Rotate(-angle) then the flip). Verify the combined variant is distinct
    // from base, rotate-only and flip-only, and round-trips back through each state to the base.
    PixmapProbe<DLatch> latch;
    const QImage baseImage = latch.displayedImage();
    QVERIFY(!baseImage.isNull());

    latch.setRotation(90);
    const QImage rotateOnly = latch.displayedImage();
    QVERIFY(rotateOnly != baseImage);

    latch.setFlippedX(true); // now rotated AND flipped
    const QImage rotateFlip = latch.displayedImage();
    QVERIFY2(rotateFlip != baseImage, "Rotated+flipped latch must differ from the base pixmap");
    QVERIFY2(rotateFlip != rotateOnly, "Rotated+flipped must differ from rotate-only");

    // ...and from a flip-only latch (flipped but not rotated).
    PixmapProbe<DLatch> flipOnly;
    flipOnly.setFlippedX(true);
    QVERIFY2(rotateFlip != flipOnly.displayedImage(), "Rotated+flipped must differ from flip-only");

    // Round-trip: unflip → back to rotate-only; then unrotate → back to the base (zero regression).
    latch.setFlippedX(false);
    QCOMPARE(latch.displayedImage(), rotateOnly);
    latch.setRotation(0);
    QCOMPARE(latch.displayedImage(), baseImage);
}

void TestCommands::testFlipNonRotatablePorts()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // --- InputButton: non-rotatable, single output port at the right edge on the mid-line ---
    // Flipping must mirror the PORT to the other side while leaving the button graphic upright
    // (no item-level transform), paralleling how rotation already repositions these ports.
    auto *button = new InputButton();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{button}, scene));
    button->setPos(100, 100);

    const QPointF portBase = button->outputPort(0)->scenePos();
    const QPointF buttonPos = button->pos();

    // Horizontal flip: the right-edge port (x=64) moves to the left edge (x=0).
    scene->receiveCommand(new FlipCommand(QList<GraphicElement *>{button}, 0, scene));
    QVERIFY2(button->transform().isIdentity(), "Non-rotatable element must not get an item flip transform");
    QCOMPARE(button->pos(), buttonPos); // lone element: the position mirror is a no-op
    const QPointF portFlipped = button->outputPort(0)->scenePos();
    QVERIFY2(portFlipped.x() < portBase.x(), "Horizontal flip must move the output port to the left side");
    QCOMPARE(portFlipped.y(), portBase.y());

    // Flip is an involution: undo restores the port exactly.
    undoStack->undo();
    QCOMPARE(button->outputPort(0)->scenePos(), portBase);

    // Vertical flip: the single port sits on the vertical centre line, so it does not move.
    scene->receiveCommand(new FlipCommand(QList<GraphicElement *>{button}, 1, scene));
    QCOMPARE(button->outputPort(0)->scenePos(), portBase);
    undoStack->undo();

    // --- Display7: overrides updatePortsProperties with off-centre, two-column ports ---
    // Guards the path where the override bypasses the base updatePortsProperties().
    auto *display = new Display7();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{display}, scene));
    display->setPos(300, 100);

    const QPointF leftPinBase = display->inputPort(0)->scenePos(); // left column (x=0)
    scene->receiveCommand(new FlipCommand(QList<GraphicElement *>{display}, 0, scene));
    QVERIFY2(display->transform().isIdentity(), "Non-rotatable display must not get an item flip transform");
    QVERIFY2(display->inputPort(0)->scenePos().x() > leftPinBase.x(),
             "Horizontal flip must move a left-column display pin to the right");
    undoStack->undo();
    QCOMPARE(display->inputPort(0)->scenePos(), leftPinBase);
}

void TestCommands::testRotateNonRotatablePorts()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // A fixed-graphic element (rotatesGraphic() == false) still rotates: setRotation keeps the
    // icon upright and repositions the ports around the centre. This is the behaviour the GUI —
    // and, after reconciliation, the MCP rotate_element handler — rely on, so a rotated
    // pushbutton's output port must actually move (and round-trip back at 0°).
    auto *button = new InputButton();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{button}, scene));
    button->setPos(100, 100);

    QVERIFY(!button->rotatesGraphic());
    const QPointF portBase = button->outputPort(0)->scenePos();

    button->setRotation(90);
    QVERIFY2(button->outputPort(0)->scenePos() != portBase,
             "Rotating a fixed-graphic element must reposition its port");

    button->setRotation(0);
    QCOMPARE(button->outputPort(0)->scenePos(), portBase);
}

void TestCommands::testFlipRotateNonRotatablePort()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // applyPortOrientation() applies rotation and flip together (order-dependent). A non-rotatable
    // element that is both rotated and flipped must place its port distinctly from rotate-only,
    // keep an identity item transform, and round-trip back through each state to the base position.
    auto *button = new InputButton();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{button}, scene));
    button->setPos(100, 100);
    QVERIFY(!button->rotatesGraphic());

    const QPointF portBase = button->outputPort(0)->scenePos();

    button->setRotation(90);
    const QPointF portRotated = button->outputPort(0)->scenePos();
    QVERIFY(portRotated != portBase);

    // applyPortOrientation applies the flip *before* the rotation (port = centre + R·S·(p−centre)),
    // so the flip acts on the pre-rotation offset. The single output port sits on the horizontal
    // centre line (x-offset, y=0), so flipX moves it (flipY would be a no-op there) — this exercises
    // the order-dependent combined rotation+flip path.
    button->setFlippedX(true);
    const QPointF portRotatedFlipped = button->outputPort(0)->scenePos();
    QVERIFY2(portRotatedFlipped != portRotated, "Flipping after rotating must move the port again");
    QVERIFY2(portRotatedFlipped != portBase, "Combined rotate+flip must differ from the base position");
    QVERIFY2(button->transform().isIdentity(), "Non-rotatable element must never get an item transform");

    // Round-trip: unflip → back to rotated; unrotate → back to base.
    button->setFlippedX(false);
    QCOMPARE(button->outputPort(0)->scenePos(), portRotated);
    button->setRotation(0);
    QCOMPARE(button->outputPort(0)->scenePos(), portBase);
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
    const auto restoredConnections = TestUtils::sceneConnections(scene);
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

void TestCommands::testMorphPreservesFlip()
{
    // Regression: morphing an element dropped its mirror state (and audio/volume/delay).
    // The flip flags must carry across the morph and survive the undo round-trip.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate}, scene));

    andGate->setFlippedX(true);
    QVERIFY(andGate->isFlippedX());
    QVERIFY(!andGate->isFlippedY());

    // Morph And -> Or: the mirror state must be transferred to the new element.
    scene->receiveCommand(new MorphCommand(scene->elements().toList(), ElementType::Or, scene));
    auto *morphed = scene->elements().first();
    QCOMPARE(morphed->elementType(), ElementType::Or);
    QVERIFY2(morphed->isFlippedX(), "Morph must preserve the horizontal flip state");
    QVERIFY(!morphed->isFlippedY());

    // Undo must restore a still-flipped And.
    undoStack->undo();
    auto *restored = scene->elements().first();
    QCOMPARE(restored->elementType(), ElementType::And);
    QVERIFY2(restored->isFlippedX(), "Undo of morph must restore the original flip state");
}

void TestCommands::testMorphPreservesVolume()
{
    // The same MorphCommand fix also carries audio/volume/delay (guarded by capability checks).
    // Volume is the morph-reachable case: Buzzer and AudioBox both have it and are 1-in/0-out, so a
    // Buzzer -> AudioBox morph must transfer the volume and restore it on undo. (Audio and delay
    // are not reachable here — no two current element types share hasAudio()/hasDelay().)
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Use a non-default volume (the default is 0.5) so this genuinely proves the carry-over rather
    // than both elements coincidentally sharing the default.
    auto *buzzer = new Buzzer();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{buzzer}, scene));
    QVERIFY(buzzer->hasVolume());
    buzzer->setVolume(0.25F);

    scene->receiveCommand(new MorphCommand(scene->elements().toList(), ElementType::AudioBox, scene));
    auto *morphed = scene->elements().first();
    QCOMPARE(morphed->elementType(), ElementType::AudioBox);
    QVERIFY(morphed->hasVolume());
    QCOMPARE(morphed->volume(), 0.25F);

    // Undo restores the Buzzer with its volume intact.
    undoStack->undo();
    auto *restored = scene->elements().first();
    QCOMPARE(restored->elementType(), ElementType::Buzzer);
    QCOMPARE(restored->volume(), 0.25F);
}

void TestCommands::testSplitCommandRedoThrowsBeforeAllocation()
{
    // Pre-fix SplitCommand::redo allocated conn2 + node BEFORE checking that
    // the upstream items still existed. When the check failed, those two
    // freshly-allocated objects were leaked. Reordering the check to run
    // first means redo throws without ever calling new.
    //
    // The throw is exercised by calling SplitCommand::redo() directly rather
    // than via QUndoStack::redo(): on macOS arm64, Qt's precompiled framework
    // lacks the compact-unwind metadata for C++ exception propagation through
    // QUndoStack::redo(), so any throw inside it calls std::terminate before
    // the catch block in the test can execute.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    sw->setPos(0, 0);
    led->setPos(200, 0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{sw, led}, scene));

    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));

    // Drive redo/undo directly on a stack-allocated command — never goes
    // through QUndoStack so the exception propagates safely on all platforms.
    SplitCommand cmd(conn, QPointF(100, 0), scene);
    cmd.redo();
    QCOMPARE(scene->elements().size(), 3);

    cmd.undo();
    QCOMPARE(scene->elements().size(), 2);

    // Make the upstream lookup fail. forgetItemId drops the registry entry
    // so findElm(m_elm1Id) returns nullptr without freeing sw itself.
    scene->forgetItemId(sw->id());

    const auto elementsBefore = scene->elements().size();
    const int connectionsBefore = TestUtils::countConnections(scene);

    bool threw = false;
    try {
        cmd.redo();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "SplitCommand::redo() must throw when upstream item lookup fails");

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

void TestCommands::testToggleTruthTableOutputCommandBounds()
{
    // Regression test (F17): the command is the model boundary shared by the
    // UI and the MCP server, and undo() == redo(). The truth-table key holds
    // exactly 2048 bits; toggleBit on any position outside [0, 2048) is an
    // out-of-bounds heap write in release builds. Out-of-range positions must
    // throw instead.
    //
    // The throws are exercised by calling redo() directly rather than via
    // QUndoStack::redo() — same macOS arm64 unwind limitation documented in
    // testSplitCommandRedoThrowsBeforeAllocation.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *truthTable = new TruthTable();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{truthTable}, scene));

    // A valid position toggles normally (and undo toggles it back).
    scene->receiveCommand(new ToggleTruthTableOutputCommand(truthTable, 5, scene));
    QVERIFY(truthTable->key().at(5));
    scene->undoStack()->undo();
    QVERIFY(!truthTable->key().at(5));

    const auto expectThrow = [&](const int position) {
        ToggleTruthTableOutputCommand command(truthTable, position, scene);
        bool threw = false;
        try {
            command.redo();
        } catch (const std::exception &) {
            threw = true;
        }
        QVERIFY2(threw, qPrintable(QString("position %1 did not throw").arg(position)));
    };

    expectThrow(2048);
    expectThrow(99999);
    expectThrow(-1);
}
