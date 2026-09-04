// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasItemSmoke.h"

#include <QSignalSpy>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

void TestCanvasItemSmoke::testConstructWithoutDemoStartsEmpty()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    QCOMPARE(canvas.elements().size(), 0);
}

void TestCanvasItemSmoke::testAddElementViaCommandUpdatesElements()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));

    QCOMPARE(canvas.elements().size(), 1);
    QCOMPARE(canvas.elements().first()->elementType(), ElementType::InputSwitch);
}

void TestCanvasItemSmoke::testAddElementUndoRemovesIt()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    QCOMPARE(canvas.elements().size(), 1);

    canvas.undoStack()->undo();

    QCOMPARE(canvas.elements().size(), 0);
}

void TestCanvasItemSmoke::testRenderMinimapImageSkipsRebuildWhenViewportUnchanged()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    (void)canvas.renderMinimapImage(200, 200);
    const int afterFirst = canvas.m_minimapRebuildCount;
    QCOMPARE(afterFirst, 1);

    (void)canvas.renderMinimapImage(200, 200);

    QCOMPARE(canvas.m_minimapRebuildCount, afterFirst);
}

void TestCanvasItemSmoke::testRenderMinimapImageRebuildsAfterStructuralEdit()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    (void)canvas.renderMinimapImage(200, 200);
    const int afterFirst = canvas.m_minimapRebuildCount;

    auto *elm2 = ElementFactory::buildElement(ElementType::And);
    elm2->setPos(500, 500);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm2}, &canvas));
    (void)canvas.renderMinimapImage(200, 200);

    QVERIFY(canvas.m_minimapRebuildCount > afterFirst);
}

void TestCanvasItemSmoke::testRenderMinimapImageRebuildsWhenZoomedOutPastCachedBounds()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    (void)canvas.renderMinimapImage(200, 200);
    const int afterFirst = canvas.m_minimapRebuildCount;

    // Zooming out grows visibleWorldRect() (more world becomes visible at once) until it no
    // longer fits inside the previously-cached content rect, forcing a real rebuild -- exercises
    // the cache-hit check's viewport-containment test, not just rebuildSpatialIndex() invalidation.
    while (canvas.canZoomOut()) {
        canvas.zoomOut();
    }
    (void)canvas.renderMinimapImage(200, 200);

    QVERIFY(canvas.m_minimapRebuildCount > afterFirst);
}

void TestCanvasItemSmoke::testScrollSizeIsFullWhenContentFitsInView()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));

    // scrollableWorldRect() unions the (empty) elementsBoundingRect() with visibleWorldRect()
    // ((0,0)-(400,400) at this size/zoom/pan) then pads by kScrollMargin (200) on every side:
    // (-200,-200)-(600,600), i.e. 800x800 -- so with nothing else on the canvas, the ratio is
    // exactly view/(view + 2*margin) = 400/800 = 0.5, not 1.0 (the margin is real headroom, not
    // a rounding artifact).
    QCOMPARE(canvas.horizontalScrollSize(), 0.5);
    QCOMPARE(canvas.verticalScrollSize(), 0.5);
}

void TestCanvasItemSmoke::testScrollSizeShrinksWhenContentExceedsView()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    const qreal baselineSize = canvas.horizontalScrollSize();

    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(10000, 10000);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    QVERIFY(canvas.horizontalScrollSize() < baselineSize);
    QVERIFY(canvas.horizontalScrollSize() < 0.1);
}

void TestCanvasItemSmoke::testSetHorizontalScrollPositionPansView()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));

    // A real, far-away element so there's something for the scrollbar to travel toward.
    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(2000, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    const qreal leftEdge = canvas.screenToWorld(QPointF(0, 0)).x();
    canvas.setHorizontalScrollPosition(1.0);
    const qreal rightEdge = canvas.screenToWorld(QPointF(0, 0)).x();

    // Dragging the scrollbar from 0.0 to 1.0 must move the view meaningfully toward the
    // far-away element, not leave it sitting near the origin. Not asserted as an exact
    // round-trip back to leftEdge afterwards: scrollableWorldRect() is re-derived from
    // wherever the view currently is on every call (see its own doc comment), so the
    // scrollable range itself shifts as the view moves -- position is a "where within the
    // range as currently seen" fraction, not a stable absolute coordinate.
    QVERIFY(rightEdge > leftEdge + 1000.0);

    canvas.setHorizontalScrollPosition(0.0);
    QVERIFY(canvas.screenToWorld(QPointF(0, 0)).x() < rightEdge);
}

void TestCanvasItemSmoke::testResizeRefreshesScrollSizeWithoutAPanOrZoom()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    QSignalSpy zoomChangedSpy(&canvas, &CanvasItem::zoomChanged);

    // visibleWorldRect()/scrollableWorldRect() read width()/height() directly in C++, a
    // dependency QML's binding tracker can't see -- so a plain resize (no zoomIn()/zoomOut()/
    // centerOn() call in sight) must still emit zoomChanged() itself, or a QML ScrollBar bound
    // to horizontalScrollSize would stay stuck at whatever it computed against this item's
    // pre-layout 0x0 size forever. See CanvasItem::geometryChange()'s own doc comment.
    canvas.setSize(QSizeF(400, 400));

    QVERIFY(zoomChangedSpy.count() >= 1);
    QCOMPARE(canvas.horizontalScrollSize(), 0.5);
}
