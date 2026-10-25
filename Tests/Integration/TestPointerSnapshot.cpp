// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestPointerSnapshot.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <QApplication>
#include <QClipboard>
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QPainterPath>
#include <QSet>
#include <QTest>
#include <QUndoStack>

#include "App/Core/Constants.h"
#include "App/Core/MimeTypes.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ClipboardManager.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace {

QString pointerGoldensDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/PointerSnapshot/";
}

QString fixturesDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Fixtures/";
}

/// Names an item by its element's id: "E<id>" for an element, "W<id>" for a wire, "P<element id>.<i|o><index>" for a
/// port, "T<type>" for any other item and "-" for none.
QString token(const QGraphicsItem *item)
{
    if (!item) {
        return QString("-");
    }
    switch (item->type()) {
    case GraphicElement::Type:
        return 'E' + QString::number(static_cast<const GraphicElement *>(item)->id());
    case Connection::Type:
        return 'W' + QString::number(static_cast<const Connection *>(item)->id());
    case Port::Type: {
        const auto *port = static_cast<const Port *>(item);
        const auto *element = port->graphicElement();
        return 'P' + QString::number(element ? element->id() : -1) + '.' + (port->isInput() ? 'i' : 'o')
               + QString::number(port->index());
    }
    default:
        return 'T' + QString::number(item->type());
    }
}

/// What a plain click lands on: the topmost element or wire whose shape holds the point.
QString clickTarget(const Scene *scene, const QPointF &pos)
{
    const auto items = scene->QGraphicsScene::items(pos, Qt::IntersectsItemShape, Qt::DescendingOrder);
    const auto target = std::find_if(items.cbegin(), items.cend(), [](const QGraphicsItem *item) {
        return (item->type() == GraphicElement::Type) || (item->type() == Connection::Type);
    });
    return token((target == items.cend()) ? nullptr : *target);
}

/// Samples sit a quarter unit off the grid lines, so none lands on an edge, where the answer would hang on rounding.
constexpr qreal kSampleOffset = 0.25;

QString cellAt(const Scene *scene, const QPointF &pos)
{
    return clickTarget(scene, pos) + '/' + token(scene->portAt(pos)) + '/' + token(scene->itemAt(pos));
}

/// Joins each run of equal neighbouring cells into "<cell>x<count>".
QString runLengths(const QStringList &cells)
{
    QStringList runs;
    qsizetype start = 0;
    while (start < cells.size()) {
        qsizetype end = start + 1;
        while ((end < cells.size()) && (cells.at(end) == cells.at(start))) {
            ++end;
        }
        runs << ((end - start == 1) ? cells.at(start) : cells.at(start) + 'x' + QString::number(end - start));
        start = end;
    }
    return runs.join(' ');
}

/// The sampled area: every element's body and every port, never the items' bounding rect, which changes when ports and
/// wires stop being items; plus a margin, widened to whole steps.
QRectF gridArea(const Scene *scene, const qreal step)
{
    qreal left = std::numeric_limits<qreal>::max();
    qreal top = left;
    qreal right = std::numeric_limits<qreal>::lowest();
    qreal bottom = right;
    const auto include = [&](const QPointF &point) {
        left = std::min(left, point.x());
        top = std::min(top, point.y());
        right = std::max(right, point.x());
        bottom = std::max(bottom, point.y());
    };

    for (const auto *element : TestUtils::elementsById(scene)) {
        const QRectF body = element->mapRectToScene(element->GraphicElement::boundingRect());
        include(body.topLeft());
        include(body.bottomRight());
        for (const auto *port : element->allPorts()) {
            include(port->scenePos());
        }
    }

    constexpr qreal margin = 16;
    return QRectF(QPointF(std::floor((left - margin) / step) * step, std::floor((top - margin) / step) * step),
                  QPointF(std::ceil((right + margin) / step) * step, std::ceil((bottom + margin) / step) * step));
}

/// A header naming the grid, then one run-length-encoded line of cells per row of samples.
QString hitGrid(const Scene *scene, const QRectF &area, const qreal step)
{
    const int columns = qRound(area.width() / step) + 1;
    const int rows = qRound(area.height() / step) + 1;
    QStringList lines;
    for (int row = 0; row < rows; ++row) {
        const qreal y = area.top() + kSampleOffset + row * step;
        QStringList cells;
        cells.reserve(columns);
        for (int column = 0; column < columns; ++column) {
            cells << cellAt(scene, QPointF(area.left() + kSampleOffset + column * step, y));
        }
        lines << "y=" + TestUtils::snapshotNumber(y) + ' ' + runLengths(cells);
    }
    return lines.join('\n') + '\n';
}

QString gridHeader(const QRectF &area, const qreal step)
{
    return QString("GRID x=%1 y=%2 step=%3 columns=%4 rows=%5\n")
        .arg(TestUtils::snapshotNumber(area.left() + kSampleOffset), TestUtils::snapshotNumber(area.top() + kSampleOffset),
             TestUtils::snapshotNumber(step), QString::number(qRound(area.width() / step) + 1),
             QString::number(qRound(area.height() / step) + 1));
}

/// The grid with everything shown, then with the gates hidden, then with the wires hidden.
QString gridsOf(Scene *scene)
{
    constexpr qreal step = 4;
    const QRectF area = gridArea(scene, step);
    QString dump = gridHeader(area, step) + "== shown\n" + hitGrid(scene, area, step);
    scene->showGates(false);
    dump += "== gates hidden\n" + hitGrid(scene, area, step);
    scene->showGates(true);
    scene->showWires(false);
    dump += "== wires hidden\n" + hitGrid(scene, area, step);
    scene->showWires(true);
    return dump;
}

GraphicElement *addElement(Scene *scene, const ElementType type, const QPointF &pos)
{
    auto *element = ElementFactory::buildElement(type);
    element->setPos(pos);
    scene->addItem(element);
    return element;
}

Connection *addWire(Scene *scene, GraphicElement *from, GraphicElement *to)
{
    auto *wire = new Connection();
    wire->setStartPort(from->outputPort(0));
    wire->setEndPort(to->inputPort(0));
    scene->addItem(wire);
    return wire;
}

/// The middle of a wire's path, in scene coordinates.
QPointF middleOf(const Connection *wire)
{
    return wire->mapToScene(wire->path().pointAtPercent(0.5));
}

/// The middle of an element's 64x64 body, clear of its ports.
QPointF bodyOf(const GraphicElement *element)
{
    return element->pos() + QPointF(32, 32);
}

/// Far from every element and wire of the gesture circuit.
constexpr QPointF kEmptySpace(-400, 400);

QString joinSorted(QStringList tokens)
{
    tokens.sort();
    return tokens.join(' ');
}

/// The selected elements and wires, as sorted tokens.
QString selection(const Scene *scene)
{
    QStringList tokens;
    for (const auto *item : scene->selectedItems()) {
        if ((item->type() == GraphicElement::Type) || (item->type() == Connection::Type)) {
            tokens << token(item);
        }
    }
    return joinSorted(tokens);
}

/// Every wire with an end on one of the scene's elements, reached through the ports.
QList<Connection *> wiresOf(const Scene *scene)
{
    QSet<Connection *> wires;
    for (const auto *element : TestUtils::elementsById(scene)) {
        for (const auto *port : element->allPorts()) {
            for (auto *wire : port->connections()) {
                wires.insert(wire);
            }
        }
    }
    return wires.values();
}

/// The wire from \a from to \a to, or null.
Connection *wireBetween(const Port *from, const Port *to)
{
    for (auto *wire : to->connections()) {
        if (wire->startPort() == from) {
            return wire;
        }
    }
    return nullptr;
}

bool isRouting(Scene *scene)
{
    return scene->connectionManager()->hasEditedConnection();
}

/// input -> gate -> led, plus an And whose ports are all free and a second Led whose input is free, far enough apart
/// that each element, wire and port is a target of its own. The input switch is never pressed, since a press toggles
/// it.
struct Circuit {
    GraphicElement *input = nullptr;
    GraphicElement *gate = nullptr;
    GraphicElement *led = nullptr;
    GraphicElement *spare = nullptr;
    GraphicElement *sink = nullptr;
    Connection *inToGate = nullptr;
    Connection *gateToLed = nullptr;
};

Circuit buildCircuit(Scene *scene)
{
    Circuit c;
    c.input = addElement(scene, ElementType::InputSwitch, QPointF(0, 0));
    c.gate = addElement(scene, ElementType::Not, QPointF(320, 160));
    c.led = addElement(scene, ElementType::Led, QPointF(640, 0));
    c.spare = addElement(scene, ElementType::And, QPointF(320, -192));
    c.sink = addElement(scene, ElementType::Led, QPointF(640, -192));
    c.inToGate = addWire(scene, c.input, c.gate);
    c.gateToLed = addWire(scene, c.gate, c.led);
    return c;
}

/// A workspace holding the gesture circuit, with rendering on so its wires have shapes to hit.
struct GestureFixture {
    TestUtils::ScopedRendering rendering;
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    Circuit c = buildCircuit(scene);
    TestUtils::ScenePointer pointer{scene};
};

} // namespace

void TestPointerSnapshot::testHitGridMatchesGolden_data()
{
    QTest::addColumn<QString>("path");
    TestUtils::addFileRows(fixturesDir(), {"*.panda"});
}

void TestPointerSnapshot::testHitGridMatchesGolden()
{
    QFETCH(QString, path);
    const QString missing = TestUtils::requireInputFile(path);
    QVERIFY2(missing.isEmpty(), qPrintable(missing));

    // With rendering off, as the suite runs, a wire builds no path and so has no shape to hit.
    const TestUtils::ScopedRendering rendering;
    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, path);
    if (QTest::currentTestFailed()) {
        return;
    }

    const QString golden = pointerGoldensDir() + "Fixtures/" + QFileInfo(path).completeBaseName() + ".txt";
    const QString mismatch = TestUtils::compareWithGolden(golden, gridsOf(workspace.scene()));
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestPointerSnapshot::testEveryGoldenHasAFixture()
{
    const auto goldens = QDir(pointerGoldensDir() + "Fixtures").entryInfoList(QDir::Files, QDir::Name);
    QVERIFY(!goldens.isEmpty());
    for (const auto &golden : goldens) {
        const QString fixture = fixturesDir() + golden.completeBaseName() + ".panda";
        QVERIFY2(QFileInfo::exists(fixture), qPrintable(golden.filePath() + " has no fixture " + fixture));
    }
}

void TestPointerSnapshot::testOverlapHitGridMatchesGolden()
{
    // Three And gates, each added above the last and shifted by less than a port's hit area, so bodies, ports and
    // the ports' hit areas overlap: where they do, Qt's click lands on the top one, and so do portAt() and itemAt(),
    // an exact hit before a nearby one.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    addElement(scene, ElementType::And, QPointF(0, 0));
    addElement(scene, ElementType::And, QPointF(8, 4));
    addElement(scene, ElementType::And, QPointF(16, 8));

    const QString mismatch = TestUtils::compareWithGolden(pointerGoldensDir() + "overlap.txt", gridsOf(scene));
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestPointerSnapshot::testErrorWireHitWidthMatchesGolden()
{
    const TestUtils::ScopedRendering rendering;
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *input = addElement(scene, ElementType::InputSwitch, QPointF(0, 0));
    auto *led = addElement(scene, ElementType::Led, QPointF(256, 96));
    auto *wire = addWire(scene, input, led);

    // A window around the wire's middle, far from both elements, sampled finely enough to show the stroke's
    // half-width: 1.5 for the 3-wide pen, 2.5 for the Error status's 5-wide one.
    constexpr qreal step = 0.5;
    const QRectF area(middleOf(wire) - QPointF(10, 10), QSizeF(20, 20));
    QString dump = gridHeader(area, step);
    for (const Status status : {Status::Inactive, Status::Error}) {
        wire->setStatus(status);
        dump += "== status " + TestUtils::snapshotStatus(wire->status()) + '\n' + hitGrid(scene, area, step);
    }

    const QString mismatch = TestUtils::compareWithGolden(pointerGoldensDir() + "error_wire.txt", dump);
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestPointerSnapshot::testClickSelectsWireAlone()
{
    GestureFixture f;

    // A click on a wire selects it alone, replacing an element and then another wire.
    f.c.gate->setSelected(true);
    f.pointer.click(middleOf(f.c.inToGate));
    QCOMPARE(selection(f.scene), token(f.c.inToGate));
    f.pointer.click(middleOf(f.c.gateToLed));
    QCOMPARE(selection(f.scene), token(f.c.gateToLed));
}

void TestPointerSnapshot::testCtrlClickOnWireAdds()
{
    GestureFixture f;

    // With no element selected, a Ctrl press on a wire adds it, and a second one leaves it selected: the scene's event
    // filter selects the item under a Ctrl press and starts a clone-drag, which returns at once without a selected
    // element.
    f.pointer.click(middleOf(f.c.inToGate), Qt::ControlModifier);
    QCOMPARE(selection(f.scene), token(f.c.inToGate));
    f.pointer.click(middleOf(f.c.gateToLed), Qt::ControlModifier);
    QCOMPARE(selection(f.scene), joinSorted({token(f.c.inToGate), token(f.c.gateToLed)}));
    f.pointer.click(middleOf(f.c.inToGate), Qt::ControlModifier);
    QCOMPARE(selection(f.scene), joinSorted({token(f.c.inToGate), token(f.c.gateToLed)}));
}

void TestPointerSnapshot::testCtrlClickOnElementAddsAndStartsCloneDrag()
{
    // On the native Windows platform QDrag::exec() first waits in its own message loop for a physical mouse move,
    // which runs no Qt timer, so nothing in the test can end the drag (startDoDragDrop() in Qt's qwindowsdrag.cpp).
    // CTest runs this class offscreen; a direct run on that platform skips the gesture rather than hang.
    if (QGuiApplication::platformName() == QLatin1String("windows")) {
        QSKIP("QDrag::exec() waits for a physical mouse move on the native Windows platform");
    }
    GestureFixture f;

    f.c.led->setSelected(true);
    f.pointer.click(bodyOf(f.c.gate), Qt::ControlModifier);
    QCOMPARE(selection(f.scene), joinSorted({token(f.c.gate), token(f.c.led)}));
    QCOMPARE(f.scene->undoStack()->count(), 0);
}

void TestPointerSnapshot::testShiftClickToggles()
{
    GestureFixture f;

    // Shift becomes Ctrl in the scene's event filter, which passes the press on to the scene's handlers.
    f.c.gate->setSelected(true);
    f.pointer.click(bodyOf(f.c.led), Qt::ShiftModifier);
    QCOMPARE(selection(f.scene), joinSorted({token(f.c.gate), token(f.c.led)}));
    f.pointer.click(bodyOf(f.c.gate), Qt::ShiftModifier);
    QCOMPARE(selection(f.scene), token(f.c.led));

    f.pointer.click(middleOf(f.c.inToGate), Qt::ShiftModifier);
    QCOMPARE(selection(f.scene), joinSorted({token(f.c.led), token(f.c.inToGate)}));
    f.pointer.click(middleOf(f.c.inToGate), Qt::ShiftModifier);
    QCOMPARE(selection(f.scene), token(f.c.led));
}

void TestPointerSnapshot::testEmptyClickClearsSelection()
{
    GestureFixture f;

    f.c.gate->setSelected(true);
    f.c.inToGate->setSelected(true);
    f.pointer.click(kEmptySpace);
    QCOMPARE(selection(f.scene), QString());

    f.pointer.click(middleOf(f.c.gateToLed));
    f.pointer.click(kEmptySpace);
    QCOMPARE(selection(f.scene), QString());
}

void TestPointerSnapshot::testPressOnElementClearsSelectedWire()
{
    GestureFixture f;

    // A press on an unselected element selects it alone, so a selected wire drops out.
    f.pointer.click(middleOf(f.c.inToGate));
    f.pointer.press(bodyOf(f.c.led));
    QCOMPARE(selection(f.scene), token(f.c.led));
    f.pointer.release(bodyOf(f.c.led));
    QCOMPARE(selection(f.scene), token(f.c.led));
}

void TestPointerSnapshot::testReleaseWithoutMovementLeavesPressedElementAlone()
{
    GestureFixture f;

    // A press on a selected element keeps the selection, ready to drag it; a release without movement then leaves that
    // element selected alone.
    f.c.gate->setSelected(true);
    f.c.led->setSelected(true);
    f.c.inToGate->setSelected(true);
    f.pointer.press(bodyOf(f.c.gate));
    QCOMPARE(selection(f.scene), joinSorted({token(f.c.gate), token(f.c.led), token(f.c.inToGate)}));
    f.pointer.release(bodyOf(f.c.gate));
    QCOMPARE(selection(f.scene), token(f.c.gate));
    QCOMPARE(f.scene->undoStack()->count(), 0);
}

void TestPointerSnapshot::testDragMovesEverySelectedElement()
{
    GestureFixture f;

    const QPointF inputFrom = f.c.input->pos();
    const QPointF gateFrom = f.c.gate->pos();
    const QPointF ledFrom = f.c.led->pos();
    f.c.gate->setSelected(true);
    f.c.led->setSelected(true);

    // Whole grid steps, so snapping leaves the pointer's delta as it is.
    const QPointF delta(4 * Constants::gridSize, 2 * Constants::gridSize);
    f.pointer.drag(bodyOf(f.c.gate), bodyOf(f.c.gate) + delta);

    QCOMPARE(f.c.gate->pos(), gateFrom + delta);
    QCOMPARE(f.c.led->pos(), ledFrom + delta);
    QCOMPARE(f.c.input->pos(), inputFrom);
    QCOMPARE(selection(f.scene), joinSorted({token(f.c.gate), token(f.c.led)}));

    // The drag is one undoable step.
    QCOMPARE(f.scene->undoStack()->count(), 1);
    f.scene->undoStack()->undo();
    QCOMPARE(f.c.gate->pos(), gateFrom);
    QCOMPARE(f.c.led->pos(), ledFrom);
}

void TestPointerSnapshot::testDragOfUnselectedElementMovesItAlone()
{
    GestureFixture f;

    const QPointF gateFrom = f.c.gate->pos();
    const QPointF ledFrom = f.c.led->pos();
    f.c.gate->setSelected(true);

    const QPointF delta(-4 * Constants::gridSize, 2 * Constants::gridSize);
    f.pointer.drag(bodyOf(f.c.led), bodyOf(f.c.led) + delta);

    QCOMPARE(f.c.led->pos(), ledFrom + delta);
    QCOMPARE(f.c.gate->pos(), gateFrom);
    QCOMPARE(selection(f.scene), token(f.c.led));
    QCOMPARE(f.scene->undoStack()->count(), 1);
}

void TestPointerSnapshot::testDragOfTopOverlappingElementIsOneUndoStep()
{
    // Where two elements overlap, Qt's press goes to the top one, which the drag moves, and the scene's own lookup,
    // itemAt(), answers the same one, so the drag records it and its move is one undoable step.
    const TestUtils::ScopedRendering rendering;
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *bottom = addElement(scene, ElementType::And, QPointF(0, 0));
    auto *top = addElement(scene, ElementType::And, QPointF(16, 16));
    const QPointF bottomFrom = bottom->pos();
    const QPointF topFrom = top->pos();
    QCOMPARE(scene->itemAt(QPointF(40, 40)), top);

    TestUtils::ScenePointer pointer(scene);
    const QPointF delta(4 * Constants::gridSize, 0);
    pointer.drag(QPointF(40, 40), QPointF(40, 40) + delta);

    QCOMPARE(top->pos(), topFrom + delta);
    QCOMPARE(bottom->pos(), bottomFrom);
    QCOMPARE(scene->undoStack()->count(), 1);
    scene->undoStack()->undo();
    QCOMPARE(top->pos(), topFrom);
    QCOMPARE(bottom->pos(), bottomFrom);
}

void TestPointerSnapshot::testPortOnTopAnswersWhereTwoAreInReach()
{
    // Two gates, the second added above the first and 8 units lower, so their output ports overlap: between them,
    // where a point is on both ports, the one drawn on top answers; on the bottom port's centre, an exact hit, it
    // answers before the top port, which is only within reach.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *bottom = addElement(scene, ElementType::And, QPointF(0, 0));
    auto *top = addElement(scene, ElementType::And, QPointF(0, 8));
    Port *bottomPort = bottom->outputPort(0);
    Port *topPort = top->outputPort(0);
    QCOMPARE(topPort->scenePos() - bottomPort->scenePos(), QPointF(0, 8));

    const QPointF between = (bottomPort->scenePos() + topPort->scenePos()) / 2.0;
    QCOMPARE(scene->portAt(between), topPort);
    QCOMPARE(scene->itemAt(between), topPort);
    QCOMPARE(scene->portAt(bottomPort->scenePos()), bottomPort);
    QCOMPARE(scene->itemAt(bottomPort->scenePos()), bottomPort);
}

void TestPointerSnapshot::testRubberBandSelectsWhatItCovers()
{
    GestureFixture f;

    // A band dragged from empty space replaces the selection with the elements and wires whose shapes it touches: here
    // the input and the first half of its wire, clear of the gate and everything else.
    f.c.led->setSelected(true);
    const QPointF from = f.c.input->pos() - QPointF(40, 40);
    const QPointF to = middleOf(f.c.inToGate) + QPointF(8, 8);
    f.pointer.drag(from, to);

    QCOMPARE(selection(f.scene), joinSorted({token(f.c.input), token(f.c.inToGate)}));
    QCOMPARE(f.scene->undoStack()->count(), 0);
}

void TestPointerSnapshot::testDragFromOutputToInputConnects()
{
    GestureFixture f;

    auto *from = f.c.spare->outputPort(0);
    auto *to = f.c.sink->inputPort(0);
    f.pointer.press(from->scenePos());
    QVERIFY(isRouting(f.scene));
    f.pointer.move((from->scenePos() + to->scenePos()) / 2.0);
    f.pointer.move(to->scenePos());
    f.pointer.release(to->scenePos());

    QVERIFY(!isRouting(f.scene));
    QVERIFY(wireBetween(from, to));
    QCOMPARE(to->connections().size(), 1);
    QCOMPARE(f.scene->undoStack()->count(), 1);
}

void TestPointerSnapshot::testClickOnOutputRoutesUntilPressOnInput()
{
    GestureFixture f;

    // A click on a port leaves the wire following the pointer; a press on a port of the other kind completes it.
    auto *from = f.c.spare->outputPort(0);
    auto *to = f.c.sink->inputPort(0);
    f.pointer.click(from->scenePos());
    QVERIFY(isRouting(f.scene));
    f.pointer.move(to->scenePos());
    QVERIFY(isRouting(f.scene));

    f.pointer.press(to->scenePos());
    QVERIFY(!isRouting(f.scene));
    QVERIFY(wireBetween(from, to));
    f.pointer.release(to->scenePos());
    QCOMPARE(to->connections().size(), 1);
    QCOMPARE(f.scene->undoStack()->count(), 1);
}

void TestPointerSnapshot::testPressOnEmptySpaceCancelsRouting()
{
    GestureFixture f;

    const auto wiresBefore = wiresOf(f.scene).size();
    f.pointer.click(f.c.spare->outputPort(0)->scenePos());
    QVERIFY(isRouting(f.scene));

    f.pointer.click(kEmptySpace);
    QVERIFY(!isRouting(f.scene));
    QCOMPARE(wiresOf(f.scene).size(), wiresBefore);
    QCOMPARE(f.scene->undoStack()->count(), 0);
}

void TestPointerSnapshot::testDragFromFreeInputToOutputConnects()
{
    GestureFixture f;

    auto *from = f.c.spare->outputPort(0);
    auto *to = f.c.sink->inputPort(0);
    f.pointer.press(to->scenePos());
    QVERIFY(isRouting(f.scene));
    f.pointer.move(from->scenePos());
    f.pointer.release(from->scenePos());

    QVERIFY(!isRouting(f.scene));
    QVERIFY(wireBetween(from, to));
    QCOMPARE(f.scene->undoStack()->count(), 1);
}

void TestPointerSnapshot::testPressOnConnectedInputDetachesWire()
{
    GestureFixture f;

    // A press on an input that has a wire picks the wire up: it leaves the input, as an undoable delete, and follows
    // the pointer from its output; a release on another input connects it there.
    auto *source = f.c.input->outputPort(0);
    auto *previous = f.c.gate->inputPort(0);
    auto *next = f.c.sink->inputPort(0);
    f.pointer.press(previous->scenePos());
    QVERIFY(previous->connections().isEmpty());
    QVERIFY(isRouting(f.scene));
    QCOMPARE(f.scene->undoStack()->count(), 1);

    f.pointer.move(next->scenePos());
    f.pointer.release(next->scenePos());
    QVERIFY(!isRouting(f.scene));
    QVERIFY(wireBetween(source, next));
    QVERIFY(previous->connections().isEmpty());
    QCOMPARE(f.scene->undoStack()->count(), 2);
}

void TestPointerSnapshot::testLoneWireDelete()
{
    GestureFixture f;

    auto *source = f.c.input->outputPort(0);
    auto *target = f.c.gate->inputPort(0);
    const qsizetype elements = f.scene->elementCount();
    f.pointer.click(middleOf(f.c.inToGate));
    QCOMPARE(selection(f.scene), token(f.c.inToGate));

    f.scene->deleteAction();
    QVERIFY(!wireBetween(source, target));
    QCOMPARE(wiresOf(f.scene).size(), 1);
    QCOMPARE(f.scene->elementCount(), elements);

    f.scene->undoStack()->undo();
    QVERIFY(wireBetween(source, target));
}

void TestPointerSnapshot::testLoneWireCopyAndCutClearClipboard()
{
    GestureFixture f;

    // Copy and cut take the selected elements; with a wire selected alone they only empty the clipboard.
    const auto clipboardHoldsCircuit = [] { return ClipboardManager::canPaste(QApplication::clipboard()->mimeData()); };
    const auto wiresBefore = wiresOf(f.scene).size();

    f.pointer.click(bodyOf(f.c.gate));
    f.scene->copyAction();
    QVERIFY(clipboardHoldsCircuit());
    f.pointer.click(middleOf(f.c.inToGate));
    f.scene->copyAction();
    QVERIFY(!clipboardHoldsCircuit());
    QCOMPARE(selection(f.scene), token(f.c.inToGate));

    f.pointer.click(bodyOf(f.c.gate));
    f.scene->copyAction();
    QVERIFY(clipboardHoldsCircuit());
    f.pointer.click(middleOf(f.c.inToGate));
    f.scene->cutAction();
    QVERIFY(!clipboardHoldsCircuit());
    QCOMPARE(selection(f.scene), token(f.c.inToGate));
    QCOMPARE(wiresOf(f.scene).size(), wiresBefore);
    QCOMPARE(f.scene->undoStack()->count(), 0);
}

void TestPointerSnapshot::testLoneWireDuplicateDoesNothing()
{
    GestureFixture f;

    const qsizetype elements = f.scene->elementCount();
    const auto wires = wiresOf(f.scene).size();
    f.pointer.click(middleOf(f.c.inToGate));
    f.scene->duplicateAction();

    QCOMPARE(f.scene->elementCount(), elements);
    QCOMPARE(wiresOf(f.scene).size(), wires);
    QCOMPARE(f.scene->undoStack()->count(), 0);
    QCOMPARE(selection(f.scene), token(f.c.inToGate));
}

void TestPointerSnapshot::testLoneWireContextMenu()
{
    // Declared before the fixture, so it outlives the scene whose requests it collects.
    QStringList requests;
    GestureFixture f;
    connect(f.scene, &Scene::contextMenuPos, this,
            [&requests](QPoint, QGraphicsItem *item) { requests << token(item); });

    // A right press on an unselected wire asks for nothing; on a selected one it asks for the selection's context
    // menu, with the wire as the item under the pointer.
    f.pointer.click(middleOf(f.c.inToGate), Qt::NoModifier, Qt::RightButton);
    QCOMPARE(requests, QStringList());
    QCOMPARE(selection(f.scene), QString());

    f.pointer.click(middleOf(f.c.inToGate));
    f.pointer.click(middleOf(f.c.inToGate), Qt::NoModifier, Qt::RightButton);
    QCOMPARE(requests, QStringList{token(f.c.inToGate)});
    QCOMPARE(selection(f.scene), token(f.c.inToGate));
}

void TestPointerSnapshot::testLoneWireSelectAll()
{
    GestureFixture f;

    f.pointer.click(middleOf(f.c.inToGate));
    f.scene->selectAll();

    QStringList everything;
    for (const auto *element : TestUtils::elementsById(f.scene)) {
        everything << token(element);
    }
    for (const auto *wire : wiresOf(f.scene)) {
        everything << token(wire);
    }
    QCOMPARE(selection(f.scene), joinSorted(everything));
}

void TestPointerSnapshot::testLoneWireZoomToSelection()
{
    GestureFixture f;
    auto *view = f.workspace.view();

    // Zoom to selection fits a wire selected alone: the view zooms in on it and leaves the far end of the circuit out.
    f.pointer.click(middleOf(f.c.inToGate));
    view->zoomToFit();

    const QRectF visible = view->mapToScene(view->viewport()->rect()).boundingRect();
    const QRectF wireEnds = QRectF(f.c.inToGate->startPort()->scenePos(), f.c.inToGate->endPort()->scenePos()).normalized();
    QVERIFY(view->transform().m11() > 1.0);
    QVERIFY(visible.contains(wireEnds));
    QVERIFY(!visible.intersects(QRectF(f.c.led->pos(), QSizeF(64, 64))));
}

void TestPointerSnapshot::testDuplicateSelectsItsCopiesWiresIncluded()
{
    GestureFixture f;

    // Duplicate leaves its copies selected, the wire between them included, and none of the originals; the copies sit
    // one grid step down and right of them.
    f.c.gate->setSelected(true);
    f.c.led->setSelected(true);
    f.c.gateToLed->setSelected(true);
    const int lastId = f.scene->lastId();
    f.scene->duplicateAction();

    QList<const GraphicElement *> copies;
    QList<const Connection *> copiedWires;
    for (const auto *item : f.scene->selectedItems()) {
        if (item->type() == GraphicElement::Type) {
            copies << static_cast<const GraphicElement *>(item);
        } else if (item->type() == Connection::Type) {
            copiedWires << static_cast<const Connection *>(item);
        }
    }
    QCOMPARE(copies.size(), 2);
    QCOMPARE(copiedWires.size(), 1);
    QCOMPARE(f.scene->undoStack()->count(), 1);

    const auto *copiedWire = copiedWires.constFirst();
    QVERIFY(copiedWire->id() > lastId);
    QVERIFY(copies.contains(copiedWire->startPort()->graphicElement()));
    QVERIFY(copies.contains(copiedWire->endPort()->graphicElement()));
    for (const auto *copy : std::as_const(copies)) {
        QVERIFY(copy->id() > lastId);
        const auto *original = (copy->elementType() == ElementType::Not) ? f.c.gate : f.c.led;
        QCOMPARE(copy->pos(), original->pos() + QPointF(Constants::gridSize, Constants::gridSize));
    }
}

void TestPointerSnapshot::testCloneDropThroughSceneEvents()
{
    // QDrag::exec() cannot be driven from a test, so the clone-drag's drop reaches the scene as the events a drop
    // sends: enter, move, then drop at the point the clone should land, carrying what ClipboardManager::cloneDrag()
    // puts in the drag (the header, the press position, the selection's centre and the selected items).
    GestureFixture f;
    f.c.gate->setSelected(true);
    const QPointF pressPos = bodyOf(f.c.gate);

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << pressPos << f.c.gate->pos();
    Serialization::serialize(f.scene->selectedItems(), stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    QMimeData mimeData;
    mimeData.setData(MimeType::CloneDrag, itemData);

    const QPointF dropPos = pressPos + QPointF(6 * Constants::gridSize, 0);
    const int lastId = f.scene->lastId();
    const qsizetype before = f.scene->elementCount();
    for (const QEvent::Type type : {QEvent::GraphicsSceneDragEnter, QEvent::GraphicsSceneDragMove, QEvent::GraphicsSceneDrop}) {
        QGraphicsSceneDragDropEvent event(type);
        event.setMimeData(&mimeData);
        event.setScenePos((type == QEvent::GraphicsSceneDragEnter) ? pressPos : dropPos);
        event.setProposedAction(Qt::CopyAction);
        event.setPossibleActions(Qt::CopyAction);
        QCoreApplication::sendEvent(f.scene, &event);
    }

    QCOMPARE(f.scene->elementCount(), before + 1);
    QCOMPARE(f.scene->undoStack()->count(), 1);
    const auto selected = f.scene->selectedElements();
    QCOMPARE(selected.size(), 1);
    QVERIFY(selected.constFirst()->id() > lastId);
    QCOMPARE(selected.constFirst()->elementType(), ElementType::Not);
    QCOMPARE(selected.constFirst()->pos(), f.c.gate->pos() + (dropPos - pressPos));
}

void TestPointerSnapshot::testPasteAddsSelectedCopies()
{
    // Paste reads the system clipboard, which is why this class holds the clipboard resource lock.
    GestureFixture f;
    f.c.gate->setSelected(true);
    f.scene->copyAction();
    QVERIFY(ClipboardManager::canPaste(QApplication::clipboard()->mimeData()));

    const qsizetype before = f.scene->elementCount();
    const int lastId = f.scene->lastId();
    f.scene->pasteAction();

    QCOMPARE(f.scene->elementCount(), before + 1);
    QCOMPARE(f.scene->undoStack()->count(), 1);
    const auto selected = f.scene->selectedElements();
    QCOMPARE(selected.size(), 1);
    QVERIFY(selected.constFirst()->id() > lastId);
    QCOMPARE(selected.constFirst()->elementType(), ElementType::Not);
}
