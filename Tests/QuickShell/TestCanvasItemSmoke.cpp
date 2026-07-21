// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasItemSmoke.h"

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

void TestCanvasItemSmoke::testMinimapZoomInWithinBoundsSkipsRebuild()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    (void)canvas.renderMinimapImage(200, 200);
    QCOMPARE(canvas.m_minimapRebuildCount, 1);

    // Zooming in strictly shrinks visibleWorldRect() -- the new viewport is guaranteed to still
    // fit inside the rect the first render already cached, regardless of direction.
    canvas.zoomIn();
    (void)canvas.renderMinimapImage(200, 200);

    QCOMPARE(canvas.m_minimapRebuildCount, 1);
}

void TestCanvasItemSmoke::testMinimapZoomOutPastBoundsForcesRebuild()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    (void)canvas.renderMinimapImage(200, 200);
    QCOMPARE(canvas.m_minimapRebuildCount, 1);

    // Zooming out enough times grows visibleWorldRect() well past what the first render cached.
    for (int i = 0; i < 5; ++i) {
        canvas.zoomOut();
    }
    (void)canvas.renderMinimapImage(200, 200);

    QCOMPARE(canvas.m_minimapRebuildCount, 2);
}

void TestCanvasItemSmoke::testMinimapStructuralEditForcesRebuild()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    (void)canvas.renderMinimapImage(200, 200);
    QCOMPARE(canvas.m_minimapRebuildCount, 1);

    auto *elm2 = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm2}, &canvas));
    (void)canvas.renderMinimapImage(200, 200);

    QCOMPARE(canvas.m_minimapRebuildCount, 2);
}
