// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasCommands.h"

#include <QDataStream>
#include <QImage>

#include "App/Core/Enums.h"
#include "App/Core/ThemeManager.h"
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

namespace {

/// Gives \a elm a custom-sized appearance image, distinguishing an edge-aware alignment
/// implementation from a bare pos()-based one (same-size elements can't tell the two apart:
/// pos() and the left/top edge coincide). Mirrors TestSceneUndoredo.cpp's identical helper.
void giveCustomSize(QTemporaryDir &dir, GraphicElement *elm, const QString &name, int width, int height)
{
    const QString path = dir.filePath(name);
    QImage image(width, height, QImage::Format_RGB32);
    image.fill(Qt::blue);
    image.save(path);
    elm->setAppearance(false, path);
}

} // namespace

void TestCanvasCommands::testAlignLeftAndTopMatchEdges()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    auto *elm3 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(96, 32);
    elm3->setPos(160, 64);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2, elm3}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);
    elm3->setSelected(true);

    // elm1 sits at the leftmost/topmost position already, so it's the alignment anchor -- its
    // own (unmoved) edge is the target, whatever that edge's exact value is (ports extend a
    // few pixels past the nominal 0/64 pixmap edges, so it isn't exactly 0.0).
    const qreal expectedLeft = elm1->sceneBoundingRect().left();
    const qreal expectedTop = elm1->sceneBoundingRect().top();

    canvas.alignLeft();
    QCOMPARE(elm1->sceneBoundingRect().left(), expectedLeft);
    QCOMPARE(elm2->sceneBoundingRect().left(), expectedLeft);
    QCOMPARE(elm3->sceneBoundingRect().left(), expectedLeft);

    canvas.undoStack()->undo(); // back to the pre-alignLeft layout before testing alignTop
    canvas.alignTop();
    QCOMPARE(elm1->sceneBoundingRect().top(), expectedTop);
    QCOMPARE(elm2->sceneBoundingRect().top(), expectedTop);
    QCOMPARE(elm3->sceneBoundingRect().top(), expectedTop);
}

void TestCanvasCommands::testAlignRightUsesSceneEdgeNotBarePos()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And); // default 64-wide
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    giveCustomSize(m_tempDir, elm2, "align_right_wide.png", 128, 64); // double width
    elm1->setPos(0, 0);
    elm2->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);

    const qreal expectedRight = std::max(elm1->sceneBoundingRect().right(), elm2->sceneBoundingRect().right());

    canvas.alignRight();

    // GraphicElement::setPos() always snaps to the canvas's 8px grid (itemChange()'s
    // ItemPositionChange-equivalent), and the two elements' right-edge-to-pos() offsets differ
    // here (ports sit exactly on the nominal left/right pixmap edges and overhang them by their
    // radius, so a differently-sized custom pixmap shifts that offset) -- the computed delta
    // isn't always an exact multiple of 8, so the moved element's edge lands within one grid
    // step of the target rather than exactly on it. Same tolerance every other
    // position-changing feature (drag, nudge, flip) already lives with.
    constexpr qreal snapTolerance = 8.0;
    QVERIFY(std::abs(elm1->sceneBoundingRect().right() - expectedRight) <= snapTolerance);
    QCOMPARE(elm2->sceneBoundingRect().right(), expectedRight); // elm2 was already the anchor; it never moved
}

void TestCanvasCommands::testAlignBottomUsesSceneEdgeNotBarePos()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And); // default 64-tall
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    giveCustomSize(m_tempDir, elm2, "align_bottom_tall.png", 64, 128); // double height
    elm1->setPos(0, 0);
    elm2->setPos(0, 200);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);

    const qreal elm1HeightBefore = elm1->sceneBoundingRect().height();
    const qreal expectedBottom = std::max(elm1->sceneBoundingRect().bottom(), elm2->sceneBoundingRect().bottom());

    canvas.alignBottom();

    QCOMPARE(elm1->sceneBoundingRect().bottom(), expectedBottom);
    QCOMPARE(elm2->sceneBoundingRect().bottom(), expectedBottom);
    QCOMPARE(elm1->pos().y(), expectedBottom - elm1HeightBefore);
    QVERIFY(elm1->pos().y() != expectedBottom);
}

void TestCanvasCommands::testAlignHorizontalCenterUsesSceneEdge()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And); // default 64-wide
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    giveCustomSize(m_tempDir, elm2, "align_center_wide.png", 128, 64); // double width
    elm1->setPos(0, 0);
    elm2->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);

    const qreal elm1PosXBefore = elm1->pos().x();
    const qreal expectedCenter = (elm1->sceneBoundingRect().center().x() + elm2->sceneBoundingRect().center().x()) / 2.0;

    canvas.alignHorizontalCenter();

    // Both elements move here (the target is the average of their original centers), so both
    // are independently subject to grid-snap rounding -- see the tolerance note in
    // testAlignRightUsesSceneEdgeNotBarePos().
    constexpr qreal snapTolerance = 8.0;
    QVERIFY(std::abs(elm1->sceneBoundingRect().center().x() - expectedCenter) <= snapTolerance);
    QVERIFY(std::abs(elm2->sceneBoundingRect().center().x() - expectedCenter) <= snapTolerance);

    // A pos()-only (buggy) implementation would instead equalize raw pos().x() values,
    // leaving elm1 near its ORIGINAL position rather than moved toward the shared center --
    // a difference far outside the grid-snap tolerance above.
    QVERIFY(std::abs(elm1->pos().x() - elm1PosXBefore) > snapTolerance);
}

void TestCanvasCommands::testAlignVerticalCenterUsesSceneEdge()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And); // default 64-tall
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    giveCustomSize(m_tempDir, elm2, "align_vcenter_tall.png", 64, 128); // double height
    elm1->setPos(0, 0);
    elm2->setPos(0, 200);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);

    const qreal elm1PosYBefore = elm1->pos().y();
    const qreal expectedCenter = (elm1->sceneBoundingRect().center().y() + elm2->sceneBoundingRect().center().y()) / 2.0;

    canvas.alignVerticalCenter();

    constexpr qreal snapTolerance = 8.0;
    QVERIFY(std::abs(elm1->sceneBoundingRect().center().y() - expectedCenter) <= snapTolerance);
    QVERIFY(std::abs(elm2->sceneBoundingRect().center().y() - expectedCenter) <= snapTolerance);

    // A pos()-only (buggy) implementation would instead equalize raw pos().y() values,
    // leaving elm1 near its ORIGINAL position rather than moved toward the shared center.
    QVERIFY(std::abs(elm1->pos().y() - elm1PosYBefore) > snapTolerance);
}

void TestCanvasCommands::testAlignNoopBelowTwoElements()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(40, 40);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    elm->setSelected(true);

    const QPointF before = elm->pos();
    const int stackCountBefore = canvas.undoStack()->count();

    canvas.alignLeft();
    canvas.alignRight();
    canvas.alignTop();
    canvas.alignBottom();
    canvas.alignHorizontalCenter();
    canvas.alignVerticalCenter();

    QCOMPARE(elm->pos(), before);
    QCOMPARE(canvas.undoStack()->count(), stackCountBefore);
}

void TestCanvasCommands::testDistributeHorizontallyEqualizesGaps()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    auto *elm3 = ElementFactory::buildElement(ElementType::And);
    // Deliberately uneven spacing before distributing -- elm2 sits far closer to elm1 than to elm3.
    elm1->setPos(0, 0);
    elm2->setPos(96, 0);
    elm3->setPos(400, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2, elm3}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);
    elm3->setSelected(true);

    const QPointF elm1PosBefore = elm1->pos();
    const QPointF elm3PosBefore = elm3->pos();

    canvas.distributeHorizontally();

    // Anchors (leftmost/rightmost) never move.
    QCOMPARE(elm1->pos(), elm1PosBefore);
    QCOMPARE(elm3->pos(), elm3PosBefore);

    const qreal gapBefore = elm2->sceneBoundingRect().left() - elm1->sceneBoundingRect().right();
    const qreal gapAfter = elm3->sceneBoundingRect().left() - elm2->sceneBoundingRect().right();
    QCOMPARE(gapBefore, gapAfter);
}

void TestCanvasCommands::testDistributeVerticallyEqualizesGaps()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    auto *elm3 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(0, 96);
    elm3->setPos(0, 400);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2, elm3}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);
    elm3->setSelected(true);

    const QPointF elm1PosBefore = elm1->pos();
    const QPointF elm3PosBefore = elm3->pos();

    canvas.distributeVertically();

    QCOMPARE(elm1->pos(), elm1PosBefore);
    QCOMPARE(elm3->pos(), elm3PosBefore);

    const qreal gapBefore = elm2->sceneBoundingRect().top() - elm1->sceneBoundingRect().bottom();
    const qreal gapAfter = elm3->sceneBoundingRect().top() - elm2->sceneBoundingRect().bottom();
    QCOMPARE(gapBefore, gapAfter);
}

void TestCanvasCommands::testDistributeNoopBelowThreeElements()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(96, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);

    const QPointF elm1Before = elm1->pos();
    const QPointF elm2Before = elm2->pos();
    const int stackCountBefore = canvas.undoStack()->count();

    canvas.distributeHorizontally();
    canvas.distributeVertically();

    QCOMPARE(elm1->pos(), elm1Before);
    QCOMPARE(elm2->pos(), elm2Before);
    QCOMPARE(canvas.undoStack()->count(), stackCountBefore);
}

void TestCanvasCommands::testAlignDistributeUndoRestoresPositions()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *elm1 = ElementFactory::buildElement(ElementType::And);
    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    auto *elm3 = ElementFactory::buildElement(ElementType::And);
    elm1->setPos(0, 0);
    elm2->setPos(96, 32);
    elm3->setPos(200, 64);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm1, elm2, elm3}, &canvas));
    elm1->setSelected(true);
    elm2->setSelected(true);
    elm3->setSelected(true);

    const QPointF pos1 = elm1->pos();
    const QPointF pos2 = elm2->pos();
    const QPointF pos3 = elm3->pos();

    canvas.alignLeft();
    QVERIFY(elm2->pos() != pos2); // sanity: something actually moved

    canvas.undoStack()->undo();

    QCOMPARE(elm1->pos(), pos1);
    QCOMPARE(elm2->pos(), pos2);
    QCOMPARE(elm3->pos(), pos3);
}

void TestCanvasCommands::testLiveThemeSwitchRefreshesElementsAndConnections()
{
    // CanvasItem::updateTheme() (element/connection loops + m_atlas.clear()) is wired to
    // ThemeManager::themeChanged directly in the constructor -- appearanceKeyFor()'s cache key
    // has no theme dimension, so without this a live theme switch would leave the atlas tile
    // (and any already-rendered element/port/connection color) stale.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw, led}, &canvas));

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    canvas.addItem(conn);

    const Theme originalTheme = ThemeManager::theme();

    // Force a known starting theme first -- Light -> Dark is guaranteed to be a real change
    // regardless of whatever theme was active before this test ran (ThemeManager is
    // process-global, shared with every other test).
    ThemeManager::setTheme(Theme::Light);
    // Port pens are deliberately theme-invariant (ThemeManager.cpp's own comment: "drawn as
    // outlines on top of the brush fill and need consistent contrast") -- brush() (kept in
    // sync with the private, status-driven currentBrush() by setCurrentBrush(), as long as no
    // hover-yellow override is active, which it isn't here) is the theme-varying color,
    // mirroring the connection colors it's designed to visually correlate with.
    const QColor lightPortColor = sw->outputPort(0)->brush().color();
    const QColor lightConnColor = conn->statusPen().color();

    ThemeManager::setTheme(Theme::Dark);
    const QColor darkPortColor = sw->outputPort(0)->brush().color();
    const QColor darkConnColor = conn->statusPen().color();

    QVERIFY2(lightPortColor != darkPortColor,
             "a live theme switch must refresh every port's status brush color");
    QVERIFY2(lightConnColor != darkConnColor,
             "a live theme switch must refresh every connection's status pen color");

    ThemeManager::setTheme(originalTheme); // don't leak state into later tests
}
