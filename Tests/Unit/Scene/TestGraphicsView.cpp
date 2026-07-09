// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestGraphicsView.h"

#include <QPainter>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestGraphicsView::testZoomMinimumLimit()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();

    for (int i = 0; i < 50; ++i)
        view->zoomOut();
    QVERIFY(!view->canZoomOut());
}

void TestGraphicsView::testZoomMaximumLimit()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();

    for (int i = 0; i < 50; ++i)
        view->zoomIn();
    QVERIFY(!view->canZoomIn());
}

void TestGraphicsView::testResetZoom()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();

    view->zoomIn();
    view->zoomIn();
    view->zoomIn();
    QVERIFY(view->canZoomOut());

    view->resetZoom();
    // After reset, should be able to zoom both in and out
    QVERIFY(view->canZoomIn());
    QVERIFY(view->canZoomOut());
}

void TestGraphicsView::testZoomToFit()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    view->resize(800, 600);

    // A circuit spread across a wide area — larger than the viewport at any zoom-in level.
    auto *inputSwitch = new InputSwitch();
    inputSwitch->setPos(0, 0);
    scene->addItem(inputSwitch);
    auto *led = new Led();
    led->setPos(3000, 2000);
    scene->addItem(led);

    // Zoom in first so the spread-out circuit no longer fits; fitting must zoom back out.
    view->zoomIn();
    view->zoomIn();
    view->zoomIn();

    view->zoomToFit();

    // The whole circuit maps within the viewport after fitting.
    const QRect itemsInView = view->mapFromScene(scene->itemsBoundingRect()).boundingRect();
    const QRect viewport = view->viewport()->rect();
    QVERIFY(itemsInView.width() <= viewport.width());
    QVERIFY(itemsInView.height() <= viewport.height());
    // Fitting a circuit far larger than the viewport must zoom out below 1:1.
    QVERIFY(view->transform().m11() < 1.0);
}

void TestGraphicsView::testFastMode()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();

    // Fast mode disables the expensive rendering hints to keep the frame rate up.
    view->setFastMode(true);
    QVERIFY(!view->renderHints().testFlag(QPainter::Antialiasing));
    QVERIFY(!view->renderHints().testFlag(QPainter::TextAntialiasing));
    QVERIFY(!view->renderHints().testFlag(QPainter::SmoothPixmapTransform));

    view->setFastMode(false);
    QVERIFY(view->renderHints().testFlag(QPainter::Antialiasing));
    QVERIFY(view->renderHints().testFlag(QPainter::TextAntialiasing));
    QVERIFY(view->renderHints().testFlag(QPainter::SmoothPixmapTransform));
}

void TestGraphicsView::testDragModeToggle()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    QCOMPARE(view->dragMode(), QGraphicsView::ScrollHandDrag);
    view->setDragMode(QGraphicsView::NoDrag);
    QCOMPARE(view->dragMode(), QGraphicsView::NoDrag);
}
