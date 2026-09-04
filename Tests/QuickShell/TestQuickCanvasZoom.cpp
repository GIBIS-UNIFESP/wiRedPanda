// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickCanvasZoom.h"

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

void TestQuickCanvasZoom::testZoomOutHitsMinimumLimit()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    for (int i = 0; i < 50; ++i) {
        canvas.zoomOut();
    }

    QVERIFY(!canvas.canZoomOut());
}

void TestQuickCanvasZoom::testZoomInHitsMaximumLimit()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    for (int i = 0; i < 50; ++i) {
        canvas.zoomIn();
    }

    QVERIFY(!canvas.canZoomIn());
}

void TestQuickCanvasZoom::testResetZoomRestoresBothDirections()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    canvas.zoomIn();
    canvas.zoomIn();
    canvas.zoomIn();
    QVERIFY(canvas.canZoomOut());

    canvas.resetZoom();

    QCOMPARE(canvas.zoomScale(), 1.0);
    QVERIFY(canvas.canZoomIn());
    QVERIFY(canvas.canZoomOut());
}

void TestQuickCanvasZoom::testZoomToFitFitsSpreadOutCircuitAndZoomsOut()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    // A circuit spread across a wide area -- larger than the viewport at any zoomed-in level.
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    inputSwitch->setPos(0, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({inputSwitch}, &canvas));
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(3000, 2000);
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas));

    // Zoom in first so the spread-out circuit no longer fits; fitting must zoom back out.
    canvas.zoomIn();
    canvas.zoomIn();
    canvas.zoomIn();

    canvas.zoomToFit();

    // The whole circuit maps within the canvas's own bounds after fitting.
    const QRectF worldRect = canvas.elementsBoundingRect();
    const QPointF screenTopLeft = canvas.worldToScreen(worldRect.topLeft());
    const QSizeF screenSize = worldRect.size() * canvas.zoomScale();
    QVERIFY2(screenTopLeft.x() >= -1.0 && screenTopLeft.y() >= -1.0,
             "zoomToFit's target rect should be positioned within the canvas, not clipped off the top-left");
    QVERIFY(screenSize.width() <= canvas.width() + 1.0);
    QVERIFY(screenSize.height() <= canvas.height() + 1.0);

    // Fitting a circuit far larger than the viewport must zoom out below 1:1.
    QVERIFY(canvas.zoomScale() < 1.0);
}
