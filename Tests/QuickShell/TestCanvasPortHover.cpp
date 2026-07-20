// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasPortHover.h"

#include <QCoreApplication>
#include <QHoverEvent>
#include <QSignalSpy>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

/// Delivers a hover event to \a canvas via QCoreApplication::sendEvent() -- same technique
/// TestCanvasItemInteraction.cpp's sendMouseEvent() uses, see that file's own doc comment for
/// why this exercises the identical code path a live gesture would.
void sendHoverEvent(CanvasItem &canvas, QEvent::Type type, const QPointF &worldPos)
{
    const QPointF screenPos = canvas.worldToScreen(worldPos);
    QHoverEvent event(type, screenPos, screenPos, screenPos, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &event);
}

void hoverAt(CanvasItem &canvas, const QPointF &worldPos)
{
    sendHoverEvent(canvas, QEvent::HoverMove, worldPos);
}

void hoverLeave(CanvasItem &canvas)
{
    sendHoverEvent(canvas, QEvent::HoverLeave, QPointF(0, 0));
}

/// Builds two elements already wired output(0)->input(0) and adds both to \a canvas via a
/// single CanvasAddItemsCommand, which auto-discovers the connection through port traversal
/// (same technique as TestCanvasCommands.cpp's own addWiredPair() helper, just a different name
/// -- an anonymous-namespace addWiredPair() defined in both files collides once unity-build
/// batching merges them into one translation unit).
void wireOutputToInput(CanvasItem &canvas, GraphicElement *source, GraphicElement *sink)
{
    auto *conn = new Connection();
    conn->setStartPort(source->outputPort(0));
    conn->setEndPort(sink->inputPort(0));
    canvas.receiveCommand(new CanvasAddItemsCommand({source, sink}, &canvas));
}

} // namespace

void TestCanvasPortHover::testHoverOverConnectedPortEmitsChipsForBothEnds()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(50, 100);
    led->setPos(250, 100);
    sw->outputPort(0)->setName("OUT");
    led->inputPort(0)->setName("IN");
    wireOutputToInput(canvas, sw, led);

    QSignalSpy spy(&canvas, &CanvasItem::portHoverChanged);

    hoverAt(canvas, led->inputPort(0)->scenePos());

    QCOMPARE(spy.count(), 1);
    const QVariantList chips = spy.constLast().constFirst().toList();
    QCOMPARE(chips.size(), 2); // the hovered port itself + its one connected peer

    const QVariantMap hovered = chips.at(0).toMap();
    QCOMPARE(hovered["text"].toString(), QStringLiteral("IN"));
    QVERIFY(hovered["radius"].toReal() > 0);
    QVERIFY(QStringList({"left", "right", "top", "bottom"}).contains(hovered["side"].toString()));

    const QVariantMap peer = chips.at(1).toMap();
    QCOMPARE(peer["text"].toString(), QStringLiteral("OUT"));
}

void TestCanvasPortHover::testHoveringSamePortTwiceDoesNotReemit()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(50, 100);
    led->setPos(250, 100);
    wireOutputToInput(canvas, sw, led);

    QSignalSpy spy(&canvas, &CanvasItem::portHoverChanged);

    const QPointF portPos = led->inputPort(0)->scenePos();
    hoverAt(canvas, portPos);
    QCOMPARE(spy.count(), 1);

    // A second hover-move at the same port (the common case: cursor jitters a pixel while
    // sitting still) must not tear down and rebuild the chip list again.
    hoverAt(canvas, portPos);
    QCOMPARE(spy.count(), 1);
}

void TestCanvasPortHover::testMovingToAnotherPortReemits()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(50, 100);
    led->setPos(250, 100);
    sw->outputPort(0)->setName("OUT");
    led->inputPort(0)->setName("IN");
    wireOutputToInput(canvas, sw, led);

    QSignalSpy spy(&canvas, &CanvasItem::portHoverChanged);

    hoverAt(canvas, led->inputPort(0)->scenePos());
    QCOMPARE(spy.count(), 1);

    hoverAt(canvas, sw->outputPort(0)->scenePos());
    QCOMPARE(spy.count(), 2);

    const QVariantList chips = spy.constLast().constFirst().toList();
    QCOMPARE(chips.size(), 2);
    QCOMPARE(chips.at(0).toMap()["text"].toString(), QStringLiteral("OUT"));
    QCOMPARE(chips.at(1).toMap()["text"].toString(), QStringLiteral("IN"));
}

void TestCanvasPortHover::testLeavingCanvasEmitsEmptyList()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(50, 100);
    led->setPos(250, 100);
    wireOutputToInput(canvas, sw, led);

    QSignalSpy spy(&canvas, &CanvasItem::portHoverChanged);

    hoverAt(canvas, led->inputPort(0)->scenePos());
    QCOMPARE(spy.count(), 1);

    hoverLeave(canvas);

    QCOMPARE(spy.count(), 2);
    QVERIFY(spy.constLast().constFirst().toList().isEmpty());
}

void TestCanvasPortHover::testUnnamedPortStillGetsChipWithEmptyText()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    // Unlike Led (which numbers its input ports "1"/"2"/... for its own multi-input display),
    // And's ports are never named by ElementInfo/updateAttrs() and stay at Port's own default
    // (an empty QString) -- neither end of this wire has a real label.
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    sw->setPos(50, 100);
    andGate->setPos(250, 100);
    wireOutputToInput(canvas, sw, andGate);

    QSignalSpy spy(&canvas, &CanvasItem::portHoverChanged);

    hoverAt(canvas, andGate->inputPort(0)->scenePos());

    QCOMPARE(spy.count(), 1);
    const QVariantList chips = spy.constLast().constFirst().toList();
    QCOMPARE(chips.size(), 2);
    // Still highlighted (a chip exists for both ends) -- just no label text to show.
    QVERIFY(chips.at(0).toMap()["text"].toString().isEmpty());
    QVERIFY(chips.at(1).toMap()["text"].toString().isEmpty());
}

void TestCanvasPortHover::testMovingToEmptySpaceClearsHover()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(50, 100);
    led->setPos(250, 100);
    wireOutputToInput(canvas, sw, led);

    QSignalSpy spy(&canvas, &CanvasItem::portHoverChanged);

    hoverAt(canvas, led->inputPort(0)->scenePos());
    QCOMPARE(spy.count(), 1);

    // Far away from either element, still inside the canvas -- a HoverMove (not HoverLeave)
    // exercises updatePortHover()'s own "port changed to nullptr" branch specifically.
    hoverAt(canvas, QPointF(700, 500));

    QCOMPARE(spy.count(), 2);
    QVERIFY(spy.constLast().constFirst().toList().isEmpty());
}
