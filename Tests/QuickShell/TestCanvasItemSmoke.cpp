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
