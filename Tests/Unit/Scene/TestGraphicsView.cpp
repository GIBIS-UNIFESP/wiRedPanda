// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestGraphicsView.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QWheelEvent>

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

void TestGraphicsView::testAccessibleNameSet()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();
    QVERIFY(!view->accessibleName().isEmpty());
    QVERIFY(!view->whatsThis().isEmpty());
}

void TestGraphicsView::testMiddleButtonPanPressAndRelease()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();

    QTest::mousePress(view->viewport(), Qt::MiddleButton, Qt::NoModifier, QPoint(100, 100));
    QCOMPARE(view->viewport()->cursor().shape(), Qt::ClosedHandCursor);

    QTest::mouseRelease(view->viewport(), Qt::MiddleButton, Qt::NoModifier, QPoint(100, 100));
    QVERIFY(view->viewport()->cursor().shape() != Qt::ClosedHandCursor);
}

void TestGraphicsView::testAutoRepeatKeyPressForwardsWithoutEnteringPanMode()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();

    QKeyEvent repeat(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QString(), /*autorep*/ true);
    QApplication::sendEvent(view, &repeat);

    // Auto-repeat must be ignored by the pan-mode logic entirely (early return before the
    // Key_Space check), so the cursor must not switch to the pan cursor.
    QVERIFY(view->viewport()->cursor().shape() != Qt::ClosedHandCursor);
}

void TestGraphicsView::testAutoRepeatKeyReleaseDoesNotExitPanMode()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();

    QKeyEvent press(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QString(), /*autorep*/ false);
    QApplication::sendEvent(view, &press);
    QCOMPARE(view->viewport()->cursor().shape(), Qt::ClosedHandCursor);

    QKeyEvent autoRepeatRelease(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, QString(), /*autorep*/ true);
    QApplication::sendEvent(view, &autoRepeatRelease);
    QCOMPARE(view->viewport()->cursor().shape(), Qt::ClosedHandCursor);

    QKeyEvent realRelease(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, QString(), /*autorep*/ false);
    QApplication::sendEvent(view, &realRelease);
    QVERIFY(view->viewport()->cursor().shape() != Qt::ClosedHandCursor);
}

void TestGraphicsView::testWheelEventZoomsOut()
{
    WorkSpace workspace;
    GraphicsView *view = workspace.view();
    view->resize(800, 600);

    const double scaleBefore = view->transform().m11();

    const QPoint center = view->viewport()->rect().center();
    QWheelEvent wheel(QPointF(center), view->viewport()->mapToGlobal(center),
                       QPoint(0, 0), QPoint(0, -120),
                       Qt::NoButton, Qt::NoModifier,
                       Qt::NoScrollPhase, false);
    QApplication::sendEvent(view->viewport(), &wheel);

    QVERIFY(view->transform().m11() < scaleBefore);
}

void TestGraphicsView::testZoomToFitWithNoSceneIsNoOp()
{
    GraphicsView view;
    QVERIFY(!view.scene());

    QSignalSpy zoomSpy(&view, &GraphicsView::zoomChanged);
    view.zoomToFit();

    QCOMPARE(zoomSpy.count(), 0);
    QCOMPARE(view.transform().m11(), 1.0);
}

void TestGraphicsView::testZoomToFitFitsSelectionOnly()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    view->resize(800, 600);

    auto *inputSwitch = new InputSwitch();
    inputSwitch->setPos(0, 0);
    scene->addItem(inputSwitch);
    auto *led = new Led();
    led->setPos(3000, 2000);
    scene->addItem(led);

    inputSwitch->setSelected(true);

    view->zoomToFit();

    // Fitting a single small selected element (ignoring the far-away Led) must zoom IN,
    // unlike fitting the whole spread-out circuit (see testZoomToFit).
    QVERIFY(view->transform().m11() > 1.0);

    const QRect switchInView = view->mapFromScene(inputSwitch->sceneBoundingRect()).boundingRect();
    QVERIFY(view->viewport()->rect().contains(switchInView));
}

void TestGraphicsView::testZoomToFitWithPlainSceneUsesItemsBoundingRect()
{
    GraphicsView view;
    view.resize(400, 300);
    QGraphicsScene plainScene;
    plainScene.addRect(0, 0, 50, 50);
    view.setScene(&plainScene);

    QSignalSpy zoomSpy(&view, &GraphicsView::zoomChanged);
    view.zoomToFit();

    // Reached the non-Scene fallback branch and completed the fit (a 50x50 rect fitted into a
    // 400x300 viewport zooms in well past 1:1).
    QCOMPARE(zoomSpy.count(), 1);
    QVERIFY(view.transform().m11() > 1.0);
}

void TestGraphicsView::testZoomToFitWithEmptySceneIsNoOp()
{
    // A real Scene always carries SceneInteraction's rubber-band selection item, so its
    // items-bounding-rect is never truly empty -- only a genuinely bare QGraphicsScene can
    // reach the invalid-target guard.
    GraphicsView view;
    view.resize(400, 300);
    QGraphicsScene emptyScene;
    view.setScene(&emptyScene);

    QSignalSpy zoomSpy(&view, &GraphicsView::zoomChanged);
    view.zoomToFit();

    QCOMPARE(zoomSpy.count(), 0);
    QCOMPARE(view.transform().m11(), 1.0);
}
