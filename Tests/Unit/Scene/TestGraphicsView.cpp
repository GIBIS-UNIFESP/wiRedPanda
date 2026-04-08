// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestGraphicsView.h"

#include "App/Scene/GraphicsView.h"
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

void TestGraphicsView::testFastMode()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();

    view->setFastMode(true);
    view->setFastMode(false);
    QVERIFY(true);
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

