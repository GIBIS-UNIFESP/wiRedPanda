// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Commands/TestCommands.h"

#include <algorithm>
#include <limits>
#include <utility>

#include <QDir>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
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

namespace TestCommandsHelpers {

/**
 * \brief Tells whether the simulation rebuilt: a switch, on, drives a Tx node an Rx node listens on. Renaming the Tx
 * directly, without a command, reroutes nothing until a rebuild reads the channels again, so the Rx still reading the
 * switch after an edit means no rebuild ran.
 */
class RebuildProbe
{
public:
    explicit RebuildProbe(Scene *scene)
    {
        CircuitBuilder builder(scene);
        builder.add(m_source, m_tx, m_rx);
        m_tx->setLabel("probe");
        m_tx->setWirelessMode(WirelessMode::Tx);
        m_rx->setLabel("probe");
        m_rx->setWirelessMode(WirelessMode::Rx);
        builder.connect(m_source, 0, m_tx, 0);
        m_source->setOn(true);
    }

    /// Renames the Tx once the simulation is built and routes the switch to the Rx; returns whether it does.
    bool arm()
    {
        const bool routed = (m_rx->outputValue(0) == Status::Active);
        m_tx->setLabel("renamed");
        return routed;
    }

    bool rebuilt() const { return m_rx->outputValue(0) != Status::Active; }

private:
    InputSwitch *m_source = new InputSwitch();
    Node *m_tx = new Node();
    Node *m_rx = new Node();
};

/// Whether \a pos lies off the half grid elements snap to.
bool offGrid(const QPointF &pos)
{
    return GraphicElement::snapToGrid(pos) != pos;
}

/**
 * \brief Returns the first circuit, in name order, of those an older release (2.3) saved, that holds an element off the
 * grid which a morph can change, with that element's id; an empty path if none does.
 */
std::pair<QString, int> offGridCase()
{
    const QDir dir(TestUtils::backwardCompatibilityDir() + "v2.3");
    for (const QString &name : dir.entryList({"*.panda"}, QDir::Files, QDir::Name)) {
        WorkSpace workspace;
        TestUtils::loadCircuit(workspace, dir.filePath(name));
        for (const auto *element : TestUtils::elementsById(workspace.scene())) {
            if (offGrid(element->pos()) && (Enums::nextElmType(element->elementType()) != ElementType::Unknown)) {
                return {dir.filePath(name), element->id()};
            }
        }
    }
    return {};
}

} // namespace TestCommandsHelpers

using TestCommandsHelpers::RebuildProbe;

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

    // setPos() snaps to the 8px half-grid (GraphicElement::itemChange), so the elements'
    // actual pre-flip positions are (104,104)/(200,104), not the raw (100,100)/(200,100)
    // passed to setPos() above. FlipCommand mirrors x about [xmin,xmax] = [104,200] and
    // leaves y untouched: gate1 104 -> 104+(200-104) = 200, gate2 200 -> 104+(200-200) = 104.
    const QPointF pos1After = gate1->pos();
    const QPointF pos2After = gate2->pos();

    QCOMPARE(pos1After, QPointF(200, 104));
    QCOMPARE(pos2After, QPointF(104, 104));

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
    // baked-in labels are counter-rotated, so the pin letters stay upright while the body rotates.
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
    button->setPos(104, 104);

    const QPointF portBase = button->outputPort(0)->scenePos();
    const QPointF buttonPos = button->pos();

    // Horizontal flip: the right-edge port (x=64) moves to the left edge (x=0).
    scene->receiveCommand(new FlipCommand(QList<GraphicElement *>{button}, 0, scene));
    QVERIFY2(button->transform().isIdentity(), "Non-rotatable element must not get an item flip transform");
    QCOMPARE(button->pos(), buttonPos); // lone element on the grid: the position mirror is a no-op
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

void TestCommands::testChangeInputSizeCommandShrinkDropsAndRestoresConnection()
{
    // Growing was already covered above; shrinking with a wire on a removed port exercises
    // a distinct, non-trivial path: redo()'s drainPortConnections() must delete the
    // connection on the removed port, and undo() must restore it.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    auto *andGate = new And();
    auto *sw = new InputSwitch();
    andGate->setPos(200, 0);
    sw->setPos(0, 0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate, sw}, scene));
    scene->receiveCommand(new ChangePortSizeCommand({andGate}, 4, scene, true));
    QCOMPARE(andGate->inputSize(), 4);

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(andGate->inputPort(3));
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));
    QVERIFY(!sw->outputPort()->connections().isEmpty());

    // Shrink back to 2 inputs: port 3 (and its connection) is dropped.
    scene->receiveCommand(new ChangePortSizeCommand({andGate}, 2, scene, true));
    QCOMPARE(andGate->inputSize(), 2);
    QVERIFY2(sw->outputPort()->connections().isEmpty(),
             "Shrinking must drop the connection on the removed input port");

    undoStack->undo();
    QCOMPARE(andGate->inputSize(), 4);
    QVERIFY2(!sw->outputPort()->connections().isEmpty(),
             "Undoing the shrink must restore the dropped connection");
    QCOMPARE(sw->outputPort()->connections().constFirst()->endPort(), andGate->inputPort(3));
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
    auto *conn1 = new Connection();
    conn1->setStartPort(sw1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));

    auto *conn2 = new Connection();
    conn2->setStartPort(sw2->outputPort());
    conn2->setEndPort(andGate->inputPort(1));

    auto *conn3 = new Connection();
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

    // Verify connections are properly connected: not just non-null endpoints (which a
    // cross-wired restore would still satisfy), but the actual expected topology -- both
    // switches landing on distinct AND input ports, and the AND output landing on the LED.
    const auto restoredConnections = TestUtils::sceneConnections(scene);
    QSet<int> andInputPortsWired;
    int andToLedConnections = 0;
    for (auto *conn : std::as_const(restoredConnections)) {
        auto *startPort = conn->startPort();
        auto *endPort = conn->endPort();
        QVERIFY2(startPort != nullptr, "Connection missing start port after undo");
        QVERIFY2(endPort != nullptr, "Connection missing end port after undo");

        auto *startElm = startPort->graphicElement();
        auto *endElm = endPort->graphicElement();
        QVERIFY(startElm);
        QVERIFY(endElm);

        if (startElm->elementType() == ElementType::InputSwitch) {
            QCOMPARE(endElm->elementType(), ElementType::And);
            andInputPortsWired.insert(endPort->index());
        } else if (startElm->elementType() == ElementType::And) {
            QCOMPARE(endElm->elementType(), ElementType::Led);
            ++andToLedConnections;
        } else {
            QFAIL("Unexpected connection start element type after undo");
        }
    }
    QCOMPARE(andInputPortsWired, QSet<int>({0, 1}));
    QCOMPARE(andToLedConnections, 1);
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

void TestCommands::testMorphCommandUndoPreservesConnectionAddedWhileMorphed()
{
    // Regression: MorphCommand::undo() didn't record connections it had to drop the way
    // redo() does (see the Display14->Display7 dangling-connection regression coverage in
    // TestSceneUndoredo) — asymmetric with the tested, documented redo-direction behavior.
    // A connection that exists on the morphed (larger) element but doesn't fit the original
    // (smaller) type used to be silently lost forever when undo() reverted the morph; it
    // must now be restored on the next redo(), symmetric with the redo-then-undo case.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    auto *disp7 = new Display7();
    auto *sw = new InputSwitch();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{disp7, sw}, scene));
    const int dispId = disp7->id();
    const int swId = sw->id();
    QCOMPARE(disp7->inputSize(), 8);

    // Morph up to Display14 (15 inputs) — pure growth, nothing dropped.
    scene->receiveCommand(new MorphCommand({disp7}, ElementType::Display14, scene));
    auto *disp14 = dynamic_cast<GraphicElement *>(scene->itemById(dispId));
    QVERIFY(disp14 != nullptr);
    QCOMPARE(disp14->elementType(), ElementType::Display14);
    QCOMPARE(disp14->inputSize(), 15);

    // Add a wire directly to port 10 — a port that exists on Display14 but not on the
    // original Display7 — bypassing the undo system, exactly as a separate user action would.
    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(disp14->inputPort(10));
    scene->addItem(conn);
    const int connId = conn->id();
    QVERIFY(!disp14->inputPort(10)->connections().isEmpty());

    // Undo the morph: Display7 has no port 10, so the connection can't survive.
    undoStack->undo();

    auto *restored7 = dynamic_cast<GraphicElement *>(scene->itemById(dispId));
    QVERIFY(restored7 != nullptr);
    QCOMPARE(restored7->elementType(), ElementType::Display7);
    QCOMPARE(restored7->inputSize(), 8);
    QVERIFY(scene->itemById(connId) == nullptr);

    // Redo: back to Display14 — the connection must be restored, not lost forever.
    undoStack->redo();

    auto *redisp14 = dynamic_cast<GraphicElement *>(scene->itemById(dispId));
    QVERIFY(redisp14 != nullptr);
    QCOMPARE(redisp14->elementType(), ElementType::Display14);
    QCOMPARE(redisp14->inputSize(), 15);
    QVERIFY(!redisp14->inputPort(10)->connections().isEmpty());
    QCOMPARE(redisp14->inputPort(10)->connections().constFirst()->startPort()->graphicElement()->id(), swId);
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

    auto *conn = new Connection();
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
    auto *conn = new Connection();
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
    andGate->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});

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

void TestCommands::testUpdateCommandPropertyEditDoesNotRebuild()
{
    // UpdateCommand covers property-only edits (label, color, delay...). It must not force a
    // full Simulation::initialize(), which rebuilds the whole simulation over an unrelated edit.
    // Regression for the bug fixed by Scene::setPropertyUpdateRequired().
    WorkSpace workspace;
    auto *scene = workspace.scene();
    RebuildProbe probe(scene);
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate}, scene));

    auto *sim = scene->simulation();
    sim->update(); // the structural edit's rebuild
    QVERIFY(probe.arm());

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    andGate->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});

    andGate->setLabel("Renamed"); // unrelated, non-structural property edit
    scene->receiveCommand(new UpdateCommand(QList<GraphicElement *>{andGate}, oldData, scene));

    sim->update(); // a rebuild requested by the command would run here
    QVERIFY(!probe.rebuilt());
}

void TestCommands::testUpdateCommandWirelessLabelRewiresChannel()
{
    // A wireless label IS the Tx→Rx channel, and the channel graph is resolved only inside
    // Simulation::initialize() (buildTxMap()/connectWirelessElements()). Renaming an Rx's
    // label via UpdateCommand must therefore escalate to a full rebuild — on the
    // property-only fast path the Rx silently keeps listening to the OLD channel until an
    // unrelated structural edit.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    CircuitBuilder builder(scene);
    auto *swA = new InputSwitch();
    auto *swB = new InputSwitch();
    auto *txA = new Node();
    auto *txB = new Node();
    auto *rx = new Node();
    builder.add(swA, swB, txA, txB, rx);
    txA->setLabel("A");
    txA->setWirelessMode(WirelessMode::Tx);
    txB->setLabel("B");
    txB->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("A");
    rx->setWirelessMode(WirelessMode::Rx);
    builder.connect(swA, 0, txA, 0);
    builder.connect(swB, 0, txB, 0);
    auto *sim = builder.initSimulation();

    swA->setOn(true);
    swB->setOn(false);
    sim->update();
    QCOMPARE(rx->outputValue(0), Status::Active); // listening on channel "A"

    // Rename the Rx's channel to "B" through the same UpdateCommand path the editor uses.
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    rx->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    rx->setLabel("B");
    scene->receiveCommand(new UpdateCommand(QList<GraphicElement *>{rx}, oldData, scene));

    sim->update();
    QCOMPARE(rx->outputValue(0), Status::Inactive); // now listening on channel "B"

    // Undo restores the old channel — again a genuine wireless change, again a rebuild.
    scene->undoStack()->undo();
    sim->update();
    QCOMPARE(rx->outputValue(0), Status::Active);
}

void TestCommands::testUpdateCommandNonWirelessNodeEditKeepsFastPath()
{
    // The wireless escalation must be precise: editing a NON-wireless property of a
    // selection that merely contains a plain (mode None) Node keeps the property-only fast
    // path — a blanket "any Node ⇒ rebuild" would reintroduce the full rebuild this
    // command's fast path exists to prevent (see testUpdateCommandPropertyEditDoesNotRebuild).
    WorkSpace workspace;
    auto *scene = workspace.scene();
    RebuildProbe probe(scene);
    auto *node = new Node(); // plain junction, WirelessMode::None
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{node}, scene));

    auto *sim = scene->simulation();
    sim->update(); // the structural edit's rebuild
    QVERIFY(probe.arm());

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    node->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    node->setLabel("junction-42"); // decorative label on a None-mode node — not a channel
    scene->receiveCommand(new UpdateCommand(QList<GraphicElement *>{node}, oldData, scene));

    sim->update(); // a rebuild requested by the command would run here
    QVERIFY(!probe.rebuilt()); // no Simulation::initialize()
}

void TestCommands::testUpdateCommandKeepsAWiredPresetOnItsWire()
{
    // An in-place reload lays the flip-flop's ports out again, setting their defaults; with the simulation stopped,
    // nothing pulls the input from its driver until a wire changes, so the wired ~Preset must stay on its wire.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    CircuitBuilder builder(scene);
    auto *preset = new InputSwitch();
    auto *flipFlop = new DFlipFlop();
    builder.add(preset, flipFlop);
    builder.connect(preset, 0, flipFlop, 2);
    auto *sim = builder.initSimulation();
    preset->setOn(false); // ~Preset asserted
    sim->update();
    QCOMPARE(flipFlop->inputPort(2)->status(), Status::Inactive);

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    flipFlop->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    flipFlop->setLabel("FF");
    scene->receiveCommand(new UpdateCommand(QList<GraphicElement *>{flipFlop}, oldData, scene));
    QCOMPARE(flipFlop->inputPort(2)->status(), Status::Inactive);

    scene->undoStack()->undo();
    QCOMPARE(flipFlop->inputPort(2)->status(), Status::Inactive);
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

    // The top edge of the valid range (2047, the last of 2048 bits) must also succeed,
    // not just an arbitrary interior position.
    scene->receiveCommand(new ToggleTruthTableOutputCommand(truthTable, 2047, scene));
    QVERIFY(truthTable->key().at(2047));
    scene->undoStack()->undo();
    QVERIFY(!truthTable->key().at(2047));

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

void TestCommands::testMorphNotToNodeAdjustsPosition()
{
    // Not and Node have different pixmap sizes; morphing between them needs a 16px
    // position adjustment so the visual center stays put.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *notGate = new Not();
    notGate->setPos(100, 100);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{notGate}, scene));
    const int id = notGate->id();

    scene->receiveCommand(new MorphCommand({notGate}, ElementType::Node, scene));
    auto *node = dynamic_cast<GraphicElement *>(scene->itemById(id));
    QVERIFY(node);
    QCOMPARE(node->elementType(), ElementType::Node);
    QCOMPARE(node->pos(), QPointF(116, 116));
}

void TestCommands::testMorphNodeToNotAdjustsPosition()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *node = ElementFactory::buildElement(ElementType::Node);
    node->setPos(100, 100);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{node}, scene));
    const int id = node->id();

    scene->receiveCommand(new MorphCommand({node}, ElementType::Not, scene));
    auto *notGate = dynamic_cast<GraphicElement *>(scene->itemById(id));
    QVERIFY(notGate);
    QCOMPARE(notGate->elementType(), ElementType::Not);
    QCOMPARE(notGate->pos(), QPointF(84, 84));
}

void TestCommands::testMorphPreservesFrequency()
{
    // Buzzer and Clock are the only two element types that both set hasFrequency in their
    // ElementInfo, so this is the only reachable pairing for this transfer branch.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    auto *buzzer = new Buzzer();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{buzzer}, scene));
    QVERIFY(buzzer->hasFrequency());
    buzzer->setFrequency(123);

    scene->receiveCommand(new MorphCommand(scene->elements().toList(), ElementType::Clock, scene));
    auto *morphed = scene->elements().first();
    QCOMPARE(morphed->elementType(), ElementType::Clock);
    QVERIFY(morphed->hasFrequency());
    QCOMPARE(morphed->frequency(), 123);

    undoStack->undo();
    auto *restored = scene->elements().first();
    QCOMPARE(restored->elementType(), ElementType::Buzzer);
    QCOMPARE(restored->frequency(), 123);
}

void TestCommands::testMorphDropsAndRestoresOutputConnection()
{
    // And has 1 output; Display7 has 0. Morphing down must drop the output connection
    // (transferPortConnections()'s output-side "port no longer exists" branch), and undo
    // must restore it via restoreDeletedConnections()'s output-side (isInput == false) path.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    auto *andGate = new And();
    auto *led = new Led();
    andGate->setPos(0, 0);
    led->setPos(200, 0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate, led}, scene));

    auto *conn = new Connection();
    conn->setStartPort(andGate->outputPort());
    conn->setEndPort(led->inputPort());
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));
    const int andId = andGate->id();

    scene->receiveCommand(new MorphCommand({andGate}, ElementType::Display7, scene));
    auto *disp7 = dynamic_cast<GraphicElement *>(scene->itemById(andId));
    QVERIFY(disp7);
    QCOMPARE(disp7->elementType(), ElementType::Display7);
    QCOMPARE(disp7->outputSize(), 0);
    // The output connection can't survive on an element with no output port.
    QVERIFY(led->inputPort(0)->connections().isEmpty());

    undoStack->undo();
    auto *restoredAnd = dynamic_cast<GraphicElement *>(scene->itemById(andId));
    QVERIFY(restoredAnd);
    QCOMPARE(restoredAnd->elementType(), ElementType::And);
    QVERIFY2(!led->inputPort(0)->connections().isEmpty(),
              "Undoing the morph must restore the dropped output connection");
    QCOMPARE(led->inputPort(0)->connections().constFirst()->startPort()->graphicElement()->id(), andId);
}

void TestCommands::testMorphUndoSkipsRestoringConnectionToDeletedPeer()
{
    // Same setup as testMorphDropsAndRestoresOutputConnection, but the peer element (led) is
    // deleted before the undo -- restoreDeletedConnections() must skip it (its own
    // "element no longer found" guard), not crash trying to dereference a stale ID.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    auto *andGate = new And();
    auto *led = new Led();
    andGate->setPos(0, 0);
    led->setPos(200, 0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate, led}, scene));

    auto *conn = new Connection();
    conn->setStartPort(andGate->outputPort());
    conn->setEndPort(led->inputPort());
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));
    const int andId = andGate->id();

    scene->receiveCommand(new MorphCommand({andGate}, ElementType::Display7, scene));

    // Delete led directly (bypassing the undo stack, like forgetItemId elsewhere in this
    // file) so the dropped connection's peer can no longer be found by ID.
    scene->removeItem(led);
    delete led;

    bool threw = false;
    try {
        undoStack->undo();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(!threw, "Undo must skip restoring a connection whose peer element is gone, not throw");

    auto *restoredAnd = dynamic_cast<GraphicElement *>(scene->itemById(andId));
    QVERIFY(restoredAnd);
    QCOMPARE(restoredAnd->elementType(), ElementType::And);
}

void TestCommands::testUpdateCommandWithNoElementsIsNoOp()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    UpdateCommand cmd({}, QByteArray(), scene);
    // loadData() on an empty elements() list must return immediately rather than trying to
    // read a header from an empty QByteArray.
    cmd.redo();
    cmd.undo();
}

void TestCommands::testUpdateBlobCommandWithNoElementsIsNoOp()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    UpdateBlobCommand cmd({}, QByteArray(), {}, scene);
    cmd.redo();
    cmd.undo();
}

void TestCommands::testFlipCommandWithEmptyItemsIsNoOp()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    FlipCommand cmd({}, 0, scene);
    QVERIFY2(cmd.text().isEmpty(), "The constructor must return before setText() for an empty selection");
    // redo()/undo() must both be safe no-ops on an empty selection, matching the sibling
    // no-op tests above (UpdateCommand/UpdateBlobCommand).
    cmd.redo();
    cmd.undo();
}

void TestCommands::testSplitCommandRedoThrowsWhenEndPortIsNull()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    sw->setPos(0, 0);
    led->setPos(200, 0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{sw, led}, scene));

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));

    SplitCommand cmd(conn, QPointF(100, 0), scene);
    cmd.redo();
    cmd.undo();

    // conn1 (the original wire) survived the undo; corrupt its endPort directly -- something
    // no current caller can do through normal interaction, but SplitCommand::redo() defends
    // against it anyway.
    conn->setEndPort(nullptr);

    bool threw = false;
    try {
        cmd.redo();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "SplitCommand::redo() must throw when the original wire's endPort is null");
}

void TestCommands::testSplitCommandUndoThrowsWhenUpstreamMissing()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    sw->setPos(0, 0);
    led->setPos(200, 0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{sw, led}, scene));

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));

    SplitCommand cmd(conn, QPointF(100, 0), scene);
    cmd.redo();

    // Make the split node unfindable without freeing it, mirroring
    // testSplitCommandRedoThrowsBeforeAllocation's forgetItemId technique.
    for (auto *elm : scene->elements()) {
        if (elm->elementType() == ElementType::Node) {
            scene->forgetItemId(elm->id());
            break;
        }
    }

    bool threw = false;
    try {
        cmd.undo();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "SplitCommand::undo() must throw when an upstream item can no longer be found");
}

void TestCommands::testOffGridElementRestoresExactly_data()
{
    QTest::addColumn<QString>("command");
    QTest::newRow("move") << "move";
    QTest::newRow("rotate") << "rotate";
    QTest::newRow("flip") << "flip";
    QTest::newRow("morph") << "morph";
}

void TestCommands::testOffGridElementRestoresExactly()
{
    // An element an older release saved off the grid: a command that decides its place puts it on the grid, its undo
    // puts it back exactly where it was, and its redo gives the same place again; a morph keeps the exact place.
    QFETCH(QString, command);
    using namespace TestCommandsHelpers;
    const auto [path, id] = offGridCase();
    QVERIFY2(!path.isEmpty(), "no circuit of release 2.3 holds a morphable element off the grid");

    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, path);
    Scene *scene = workspace.scene();
    const auto element = [scene, elementId = id] { return dynamic_cast<GraphicElement *>(scene->itemById(elementId)); };
    QVERIFY(element());
    const QPointF start = element()->pos();
    QVERIFY(offGrid(start));

    if (command == "move") {
        element()->setPos(GraphicElement::snapToGrid(start + QPointF(40, 24)));
        scene->receiveCommand(new MoveCommand({element()}, {start}, scene));
    } else if (command == "rotate") {
        scene->receiveCommand(new RotateCommand({element()}, 90, scene));
    } else if (command == "flip") {
        scene->receiveCommand(new FlipCommand({element()}, 0, scene));
    } else {
        scene->receiveCommand(new MorphCommand({element()}, Enums::nextElmType(element()->elementType()), scene));
    }

    QVERIFY(element());
    const QPointF done = element()->pos();
    QVERIFY2((command == "morph") ? (done == start) : !offGrid(done),
             qPrintable(QString("%1 left the element at %2,%3").arg(command).arg(done.x()).arg(done.y())));

    scene->undoStack()->undo();
    QVERIFY(element());
    QCOMPARE(element()->pos(), start);

    scene->undoStack()->redo();
    QVERIFY(element());
    QCOMPARE(element()->pos(), done);
}

void TestCommands::testSetPosIsExact()
{
    // Setting a position in a scene keeps it as given; only the edits that decide a place snap.
    WorkSpace workspace;
    auto *gate = new And();
    workspace.scene()->addItem(gate);
    gate->setPos(279.333, 311.5);
    QCOMPARE(gate->pos(), QPointF(279.333, 311.5));
}

void TestCommands::testFindItemsThrowsWhenItemMissing()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = new And();
    AddItemsCommand cmd(QList<QGraphicsItem *>{andGate}, scene);

    // Drop the registry entry without freeing the object, so findItems() can't resolve the
    // ID -- mirrors testSplitCommandRedoThrowsBeforeAllocation's forgetItemId technique.
    scene->forgetItemId(andGate->id());

    bool threw = false;
    try {
        cmd.undo();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "AddItemsCommand::undo() must throw when a tracked item can no longer be found");
}

void TestCommands::testLoadItemsThrowsOnCountMismatch()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = new And();
    scene->addItem(andGate);

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    // Serialization::serialize() (not elm->save() directly) writes the leading type tag
    // Serialization::deserialize() expects, so this parses as exactly one real item --
    // otherwise the mismatch below would be masked by an earlier stream-parsing error.
    Serialization::serialize({andGate}, stream, {.purpose = SerializationPurpose::InMemorySnapshot});

    // Claim two IDs are expected, but the stream only holds one serialized element.
    const QList<int> ids{1, 2};
    QList<int> otherIds;

    bool threw = false;
    try {
        CommandUtils::loadItems(scene, itemData, ids, otherIds);
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "loadItems() must throw when the deserialized item count doesn't match the expected ID count");
}

void TestCommands::testElementsCommandElementsThrowsWhenItemMissing()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate}, scene));

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    andGate->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});

    UpdateCommand cmd({andGate}, oldData, scene);
    scene->forgetItemId(andGate->id());

    bool threw = false;
    try {
        cmd.redo();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "ElementsCommand::elements() must throw when a tracked element can no longer be found");
}

void TestCommands::testMorphUndoRestoresWhatTheNewTypeLacks()
{
    // A switch has no frequency or delay, and a morph does not carry the lock: undo brings the clock back as it was
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *clock = new Clock();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{clock}, scene));
    clock->setFrequency(5.0);
    clock->setDelay(0.25);
    clock->setLocked(true);
    clock->setLabel("clk");
    const int id = clock->id();

    scene->receiveCommand(new MorphCommand({clock}, ElementType::InputSwitch, scene));
    QCOMPARE(dynamic_cast<GraphicElement *>(scene->itemById(id))->elementType(), ElementType::InputSwitch);

    scene->undoStack()->undo();
    auto *restored = dynamic_cast<Clock *>(scene->itemById(id));
    QVERIFY(restored);
    QCOMPARE(restored->frequency(), 5.0);
    QCOMPARE(restored->delay(), 0.25);
    QVERIFY(restored->isLocked());
    QCOMPARE(restored->label(), QString("clk"));

    // Redo morphs the restored clock again
    scene->undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(scene->itemById(id))->elementType(), ElementType::InputSwitch);
    QCOMPARE(dynamic_cast<GraphicElement *>(scene->itemById(id))->label(), QString("clk"));
}

void TestCommands::testMorphUndoRestoresInputsTheNewTypeCannotHave()
{
    // A Not has one input: morphing a four-input And drops three wires, and undo restores the four inputs with them
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate}, scene));
    andGate->setInputSize(4);
    for (int i = 0; i < 4; ++i) {
        auto *vcc = new InputVcc();
        vcc->setPos(-128, i * 64);
        scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{vcc}, scene));
        auto *conn = new Connection();
        conn->setStartPort(vcc->outputPort());
        conn->setEndPort(andGate->inputPort(i));
        scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));
    }
    const int id = andGate->id();

    scene->receiveCommand(new MorphCommand({andGate}, ElementType::Not, scene));
    QCOMPARE(dynamic_cast<GraphicElement *>(scene->itemById(id))->inputSize(), 1);

    scene->undoStack()->undo();
    auto *restored = dynamic_cast<GraphicElement *>(scene->itemById(id));
    QVERIFY(restored);
    QCOMPARE(restored->elementType(), ElementType::And);
    QCOMPARE(restored->inputSize(), 4);
    for (int i = 0; i < 4; ++i) {
        QCOMPARE(restored->inputPort(i)->connections().size(), 1);
    }
}

void TestCommands::testMorphUndoWithAPictureThatIsGoneShowsTheDefault()
{
    // Undo restores the saved switch, whose custom picture no longer loads: the switch comes back drawing its slot's
    // default, keeps the path, and the stack steps
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString png = dir.filePath("custom.png");
    QImage red(64, 64, QImage::Format_ARGB32);
    red.fill(Qt::red);
    QVERIFY(red.save(png));

    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *inputSwitch = new InputSwitch();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{inputSwitch}, scene));
    inputSwitch->setAppearance(false, png);
    const int id = inputSwitch->id();

    scene->receiveCommand(new MorphCommand({inputSwitch}, ElementType::InputButton, scene));
    QVERIFY(QFile::remove(png));

    scene->undoStack()->undo();
    QCOMPARE(scene->undoStack()->index(), 1);
    QCOMPARE(scene->elements().size(), 1);
    auto *restored = dynamic_cast<GraphicElement *>(scene->itemById(id));
    QCOMPARE(restored->elementType(), ElementType::InputSwitch);
    QCOMPARE(restored->externalFiles(), QStringList{png});
    const InputSwitch fresh;
    QCOMPARE(SnapshotAccess::pixmapIdentity(restored), SnapshotAccess::pixmapIdentity(&fresh));
}

void TestCommands::testUndoWithAPictureThatIsGoneShowsTheDefault()
{
    // A reset to the default picture is undone after the custom picture's file is gone: the undo restores the path,
    // the switch draws its slot's default, and the stack steps; the redo steps back
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString png = dir.filePath("custom.png");
    QImage red(64, 64, QImage::Format_ARGB32);
    red.fill(Qt::red);
    QVERIFY(red.save(png));

    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *inputSwitch = new InputSwitch();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{inputSwitch}, scene));
    inputSwitch->setAppearance(false, png);

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    inputSwitch->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    inputSwitch->setAppearance(true, {});
    scene->receiveCommand(new UpdateCommand({inputSwitch}, oldData, scene));
    QVERIFY(QFile::remove(png));

    scene->undoStack()->undo();
    QCOMPARE(scene->undoStack()->index(), 1);
    QCOMPARE(inputSwitch->externalFiles(), QStringList{png});
    const InputSwitch fresh;
    QCOMPARE(SnapshotAccess::pixmapIdentity(inputSwitch), SnapshotAccess::pixmapIdentity(&fresh));

    scene->undoStack()->redo();
    QCOMPARE(scene->undoStack()->index(), 2);
    QVERIFY(inputSwitch->externalFiles().isEmpty());
}

void TestCommands::testUndoWithASoundThatIsGoneKeepsItsPath()
{
    // A change to the built-in sound is undone after the custom sound's file is gone: the undo restores the path, the
    // box plays its default, and the stack steps; the redo steps back
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString wav = dir.filePath("custom.wav");
    QFile file(wav);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("DUMMY AUDIO DATA");
    file.close();

    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *audioBox = new AudioBox();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{audioBox}, scene));
    audioBox->setAudio(wav);

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    audioBox->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    audioBox->setAudio(QString::fromLatin1(AudioBox::kDefaultAudioPath));
    scene->receiveCommand(new UpdateCommand({audioBox}, oldData, scene));
    QVERIFY(QFile::remove(wav));

    scene->undoStack()->undo();
    QCOMPARE(scene->undoStack()->index(), 1);
    QCOMPARE(audioBox->audio(), wav);

    scene->undoStack()->redo();
    QCOMPARE(scene->undoStack()->index(), 2);
    QCOMPARE(audioBox->audio(), QString::fromLatin1(AudioBox::kDefaultAudioPath));
}

void TestCommands::testSetPropertyCommandUndoesEachElementsValue()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *red = new Led();
    auto *green = new Led();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{red, green}, scene));
    red->setColor("Red");
    green->setColor("Green");

    auto *command = SetPropertyCommand::apply({red, green}, ElementProperty::Color, QString("Blue"), scene);
    QVERIFY(command != nullptr);
    QCOMPARE(red->color(), QString("Blue"));
    QCOMPARE(green->color(), QString("Blue"));
    scene->receiveCommand(command);
    QCOMPARE(command->text(), QString("Update 2 elements"));

    scene->undoStack()->undo();
    QCOMPARE(red->color(), QString("Red"));
    QCOMPARE(green->color(), QString("Green"));
    scene->undoStack()->redo();
    QCOMPARE(red->color(), QString("Blue"));
    QCOMPARE(green->color(), QString("Blue"));
}

void TestCommands::testSetPropertyCommandNamesEveryElementThatHasTheProperty()
{
    // A gate has no label and is left out; a switch that already has the value stays in, so every step of an edit
    // session names the same elements. An edit that changes no value is no command.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *gate = new And();
    auto *named = new InputSwitch();
    auto *other = new InputSwitch();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{gate, named, other}, scene));
    named->setLabel("x");

    QCOMPARE(SetPropertyCommand::apply({gate, named}, ElementProperty::Label, QString("x"), scene), nullptr);
    QCOMPARE(gate->label(), QString());

    auto *command = SetPropertyCommand::apply({gate, named, other}, ElementProperty::Label, QString("x"), scene);
    QVERIFY(command != nullptr);
    QCOMPARE(gate->label(), QString());
    QCOMPARE(other->label(), QString("x"));
    scene->receiveCommand(command);
    QCOMPARE(command->text(), QString("Update 2 elements"));

    scene->undoStack()->undo();
    QCOMPARE(named->label(), QString("x"));
    QCOMPARE(other->label(), QString());
}

void TestCommands::testSetPropertyCommandSetsAnInputsLockAndValue()
{
    // A rotary switch's value is the output it selects, a switch's its level
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *inputSwitch = new InputSwitch();
    auto *rotary = new InputRotary();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{inputSwitch, rotary}, scene));
    rotary->setOutputSize(4);

    scene->receiveCommand(SetPropertyCommand::apply({inputSwitch, rotary}, ElementProperty::Value, 2, scene));
    QVERIFY(inputSwitch->isOn());
    QCOMPARE(rotary->outputValue(), 2);
    scene->receiveCommand(SetPropertyCommand::apply({inputSwitch, rotary}, ElementProperty::Locked, true, scene));
    QVERIFY(inputSwitch->isLocked());
    QVERIFY(rotary->isLocked());

    scene->undoStack()->undo();
    QVERIFY(!inputSwitch->isLocked());
    QVERIFY(!rotary->isLocked());
    scene->undoStack()->undo();
    QVERIFY(!inputSwitch->isOn());
    QCOMPARE(rotary->outputValue(), 0);
}

void TestCommands::testSetPropertyCommandFirstRedoLeavesTheValues()
{
    // apply() set the values; pushing the command writes nothing again
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *input = new InputSwitch();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{input}, scene));

    auto *command = SetPropertyCommand::apply({input}, ElementProperty::Label, QString("a"), scene);
    QVERIFY(command != nullptr);
    input->setLabel("changed meanwhile");
    scene->receiveCommand(command);
    QCOMPARE(input->label(), QString("changed meanwhile"));

    scene->undoStack()->undo();
    QCOMPARE(input->label(), QString());
    scene->undoStack()->redo();
    QCOMPARE(input->label(), QString("a"));
}

void TestCommands::testSetPropertyCommandMergesASession()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();
    auto *input = new InputSwitch();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{input}, scene));
    input->setLabel("start");
    const int commands = undoStack->count();

    const auto push = [&](const QString &text, const int session) {
        scene->receiveCommand(SetPropertyCommand::apply({input}, ElementProperty::Label, text, scene, session));
    };
    push("a", 7);
    push("ab", 7);
    push("abc", 7);
    QCOMPARE(undoStack->count(), commands + 1);

    // Another session, and no session, start a step of their own
    push("abcd", 8);
    QCOMPARE(undoStack->count(), commands + 2);
    push("x", -1);
    push("xy", -1);
    QCOMPARE(undoStack->count(), commands + 4);

    undoStack->undo();
    undoStack->undo();
    undoStack->undo();
    QCOMPARE(input->label(), QString("abc"));
    undoStack->undo();
    QCOMPARE(input->label(), QString("start"));
    undoStack->redo();
    QCOMPARE(input->label(), QString("abc"));
}

void TestCommands::testSetPropertyCommandMergedBackIsNoStep()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();
    auto *clock = new Clock();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{clock}, scene));
    const double frequency = clock->frequency();
    const int commands = undoStack->count();

    scene->receiveCommand(SetPropertyCommand::apply({clock}, ElementProperty::Frequency, frequency + 1.0, scene, 3));
    QCOMPARE(undoStack->count(), commands + 1);
    scene->receiveCommand(SetPropertyCommand::apply({clock}, ElementProperty::Frequency, frequency, scene, 3));
    QCOMPARE(undoStack->count(), commands);
    QCOMPARE(clock->frequency(), frequency);
}

void TestCommands::testSetPropertyCommandMergesOnlyTheSameChanges()
{
    // One session's edits of another property, or of other elements, are steps of their own
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();
    auto *first = new Clock();
    auto *second = new Clock();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{first, second}, scene));
    const int commands = undoStack->count();

    scene->receiveCommand(SetPropertyCommand::apply({first}, ElementProperty::Frequency, 2.0, scene, 5));
    scene->receiveCommand(SetPropertyCommand::apply({first}, ElementProperty::Label, QString("c"), scene, 5));
    QCOMPARE(undoStack->count(), commands + 2);
    scene->receiveCommand(SetPropertyCommand::apply({second}, ElementProperty::Label, QString("s"), scene, 5));
    QCOMPARE(undoStack->count(), commands + 3);
    scene->receiveCommand(SetPropertyCommand::apply({first, second}, ElementProperty::Label, QString("d"), scene, 5));
    QCOMPARE(undoStack->count(), commands + 4);
    scene->receiveCommand(SetPropertyCommand::apply({second}, ElementProperty::Label, QString("e"), scene, 5));
    QCOMPARE(undoStack->count(), commands + 5);

    undoStack->undo();
    QCOMPARE(second->label(), QString("d"));
    undoStack->undo();
    QCOMPARE(first->label(), QString("c"));
    QCOMPARE(second->label(), QString("s"));
    undoStack->undo();
    QCOMPARE(first->label(), QString("c"));
    QCOMPARE(second->label(), QString());
    undoStack->undo();
    QCOMPARE(first->label(), QString());
    QCOMPARE(first->frequency(), 2.0);
}

void TestCommands::testSetPropertyCommandRxLabelRewiresChannel()
{
    // An Rx's label is its channel, which the simulation resolves only as it is rebuilt
    WorkSpace workspace;
    auto *scene = workspace.scene();
    CircuitBuilder builder(scene);
    auto *swA = new InputSwitch();
    auto *swB = new InputSwitch();
    auto *txA = new Node();
    auto *txB = new Node();
    auto *rx = new Node();
    builder.add(swA, swB, txA, txB, rx);
    txA->setLabel("A");
    txA->setWirelessMode(WirelessMode::Tx);
    txB->setLabel("B");
    txB->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("A");
    rx->setWirelessMode(WirelessMode::Rx);
    builder.connect(swA, 0, txA, 0);
    builder.connect(swB, 0, txB, 0);
    auto *sim = builder.initSimulation();

    swA->setOn(true);
    swB->setOn(false);
    sim->update();
    QCOMPARE(rx->outputValue(0), Status::Active);

    scene->receiveCommand(SetPropertyCommand::apply({rx}, ElementProperty::Label, QString("B"), scene));
    sim->update();
    QCOMPARE(rx->outputValue(0), Status::Inactive);

    scene->undoStack()->undo();
    sim->update();
    QCOMPARE(rx->outputValue(0), Status::Active);
}

void TestCommands::testSetPropertyCommandNonWirelessLabelDoesNotRebuild()
{
    // A label of a node with no mode names no channel: no rebuild
    WorkSpace workspace;
    auto *scene = workspace.scene();
    RebuildProbe probe(scene);
    auto *node = new Node();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{node}, scene));
    auto *sim = scene->simulation();
    sim->update(); // the structural edit's rebuild
    QVERIFY(probe.arm());

    scene->receiveCommand(SetPropertyCommand::apply({node}, ElementProperty::Label, QString("junction"), scene));
    sim->update();
    QVERIFY(!probe.rebuilt());
    scene->undoStack()->undo();
    sim->update();
    QVERIFY(!probe.rebuilt());
}

void TestCommands::testLoadListGrowsLinearly()
{
    // loadList() is what every add, delete and paste command runs over its items, deduplicating them and checking each
    // wire's far end against the ids, which must stay linear in the items. Walking thousands of scattered items grows
    // faster than n by itself, from cache misses, so loadList() is timed against a walk of the same items, ports and
    // wires that keeps everything: how that ratio grows from 250 elements to 32,000, 0.8 to 1.05 for a linear
    // loadList() and 11 to 25 for a quadratic one, with eight copies of the test running at once. The sizes are far
    // apart so that a loaded runner's noise stays well under the quadratic growth. Ratios of the thread's CPU time
    // (TestUtils::ThreadCpuTimer), each the best of nine runs.
    struct Times {
        qint64 loadList = 0;
        qint64 walk = 0;
    };
    const auto bestTimes = [](const int n) {
        WorkSpace workspace;
        auto *scene = workspace.scene();
        GraphicElement *previous = nullptr;
        for (int i = 0; i < n; ++i) {
            auto *notGate = ElementFactory::buildElement(ElementType::Not);
            scene->addItem(notGate);
            if (previous) {
                auto *conn = new Connection();
                conn->setStartPort(previous->outputPort());
                conn->setEndPort(notGate->inputPort());
                scene->addItem(conn);
            }
            previous = notGate;
        }
        const auto items = scene->items();

        Times best{std::numeric_limits<qint64>::max(), std::numeric_limits<qint64>::max()};
        for (int run = 0; run < 9; ++run) {
            QList<int> ids;
            QList<int> otherIds;
            TestUtils::ThreadCpuTimer timer;
            timer.start();
            const auto listed = CommandUtils::loadList(items, ids, otherIds);
            best.loadList = std::min(best.loadList, timer.elapsed());
            if (listed.size() != (2 * n) - 1) {
                return Times{};
            }

            QList<QGraphicsItem *> walked;
            QList<int> walkedIds;
            timer.start();
            for (auto *item : items) {
                if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
                    walked.append(item);
                    walkedIds.append(elm->id());
                    for (auto *port : elm->inputs()) {
                        for (auto *conn : port->connections()) {
                            walked.append(conn);
                        }
                    }
                    for (auto *port : elm->outputs()) {
                        for (auto *conn : port->connections()) {
                            walked.append(conn);
                        }
                    }
                }
            }
            best.walk = std::min(best.walk, timer.elapsed());
            if (walked.isEmpty() || walkedIds.isEmpty()) {
                return Times{};
            }
        }
        return best;
    };

    const Times small = bestTimes(250);
    const Times large = bestTimes(32000);
    QVERIFY2((small.loadList > 0) && (large.loadList > 0) && (small.walk > 0) && (large.walk > 0),
             "loadList() lost or added items");
    const double growth = (static_cast<double>(large.loadList) / static_cast<double>(large.walk))
                        / (static_cast<double>(small.loadList) / static_cast<double>(small.walk));
    // Reported on every run, so a runner's spread can be read from its logs, a pass's as well as a failure's
    const QString times = QString("loadList() %1 and the walk %2 %5 at 250 elements, %3 and %4 %5 at 32,000")
                              .arg(small.loadList).arg(small.walk).arg(large.loadList).arg(large.walk)
                              .arg(TestUtils::ThreadCpuTimer::unit());
    qInfo().noquote() << QString("growth %1; %2").arg(growth).arg(times);
    QVERIFY2(growth < 4.0, qPrintable(QString("against a walk of the same items, loadList() took %1 times as long "
                                              "at 128 times the elements (%2)").arg(growth).arg(times)));
}
