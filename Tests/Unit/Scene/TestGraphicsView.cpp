// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestGraphicsView.h"

#include <utility>

#include <QApplication>
#include <QCursor>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSignalSpy>
#include <QWheelEvent>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace {

/**
 * \brief Gives \a view an 800x600 viewport on every platform.
 * \details Each platform's style draws its own frame and scroll bars, which would take their widths from the viewport.
 * With neither, the viewport is 800x600 everywhere; the hidden scroll bars still hold the scroll position and its
 * clamp. A hidden widget defers its resize event to show(), and the view lays out its viewport and its scroll ranges
 * on that event, so it is sent.
 */
void layOutView(GraphicsView *view)
{
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->resize(800, 600);
    QResizeEvent resize(view->size(), QSize());
    QCoreApplication::sendEvent(view, &resize);
}

/// The zoom, the scroll bars' positions, the scene point at the viewport's centre, and whether each zoom direction is
/// open.
QString viewLine(const GraphicsView *view, const QString &label)
{
    const QPointF centre = view->mapToScene(view->viewport()->rect().center());
    return QString("%1 m11=%2 scroll=%3,%4 centre=%5 canIn=%6 canOut=%7")
        .arg(label, TestUtils::snapshotNumber(view->transform().m11()), QString::number(view->horizontalScrollBar()->value()),
             QString::number(view->verticalScrollBar()->value()), TestUtils::snapshotPoint(centre),
             QString::number(view->canZoomIn()), QString::number(view->canZoomOut()));
}

/// A wheel notch over the viewport at \a cursor, \a delta 120 to zoom in or -120 to zoom out, and its row: the view
/// after it, and the scene point under the cursor before and after.
QStringList wheelRows(GraphicsView *view, const QString &label, const QPoint cursor, const int delta)
{
    const QPointF before = view->mapToScene(cursor);
    QWheelEvent wheel(QPointF(cursor), view->viewport()->mapToGlobal(cursor), QPoint(0, 0), QPoint(0, delta), Qt::NoButton,
                      Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(view->viewport(), &wheel);
    return {viewLine(view, label + " at " + QString::number(cursor.x()) + ',' + QString::number(cursor.y())),
            "  cursor scene before " + TestUtils::snapshotPoint(before) + " after "
                + TestUtils::snapshotPoint(view->mapToScene(cursor))};
}

} // namespace

void TestGraphicsView::testViewBehavioursMatchGolden()
{
    // Three elements spread wider than an 800x600 view, so zooming, fitting and scrolling all have work to do.
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    const QList<std::pair<ElementType, QPointF>> placements{
        {ElementType::InputSwitch, QPointF(40, 40)},
        {ElementType::And, QPointF(900, 300)},
        {ElementType::Led, QPointF(-400, 700)},
    };
    for (const auto &[type, position] : placements) {
        auto *element = ElementFactory::buildElement(type);
        element->setPos(position);
        scene->addItem(element);
    }
    layOutView(view);
    QCOMPARE(view->viewport()->size(), QSize(800, 600));

    QStringList lines{viewLine(view, "initial")};
    for (int step = 1; (step <= 50) && view->canZoomIn(); ++step) {
        view->zoomIn();
        lines << viewLine(view, "in " + QString::number(step));
    }
    for (int step = 1; (step <= 50) && view->canZoomOut(); ++step) {
        view->zoomOut();
        lines << viewLine(view, "out " + QString::number(step));
    }

    view->zoomToFit();
    lines << viewLine(view, "fit");
    for (const QPointF centre : {QPointF(0, 0), QPointF(300, 200), QPointF(-350, 650)}) {
        view->centerOn(centre);
        lines << viewLine(view, "centerOn " + TestUtils::snapshotPoint(centre));
    }
    for (const QRectF target : {QRectF(500, 400, 50, 50), QRectF(-420, -60, 30, 30)}) {
        view->ensureVisible(target);
        lines << viewLine(view, "ensureVisible " + TestUtils::snapshotPoint(target.topLeft()));
    }

    // A wheel notch keeps the scene point under the cursor under it, as each row's before and after show.
    view->resetZoom();
    lines << viewLine(view, "reset");
    lines << wheelRows(view, "wheel in", QPoint(120, 90), 120);
    lines << wheelRows(view, "wheel out", QPoint(120, 90), -120);
    lines << wheelRows(view, "wheel in", QPoint(780, 580), 120);
    lines << wheelRows(view, "wheel out", QPoint(400, 300), -120);

    const QString mismatch = TestUtils::compareWithGolden(QString(QUOTE(CURRENTDIR)) + "/Integration/GraphicsView/behaviours.txt",
                                                          lines.join('\n') + '\n');
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

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

void TestGraphicsView::testWheelKeepsCursorPoint()
{
    // With room to scroll, a wheel zoom in either direction leaves the scene point under the cursor under it, both
    // with the pointer away, where a button's zoom pivots on the viewport's centre, and with it over the view, where
    // it pivots on the pointer.
    WorkSpace workspace;
    GraphicsView *view = workspace.view();
    workspace.scene()->setSceneRect(QRectF(-1e5, -1e5, 2e5, 2e5));
    layOutView(view);
    const QPoint cursor(120, 90);

    for (const bool pointerOver : {false, true}) {
        view->setAttribute(Qt::WA_UnderMouse, pointerOver);
        if (pointerOver) {
            QCursor::setPos(view->viewport()->mapToGlobal(cursor));
            QMouseEvent move(QEvent::MouseMove, QPointF(cursor), view->viewport()->mapToGlobal(QPointF(cursor)), Qt::NoButton,
                             Qt::NoButton, Qt::NoModifier);
            QApplication::sendEvent(view->viewport(), &move);

            // The pivot is the pointer's point: a button's zoom keeps it, as the wheel's does.
            view->resetZoom();
            view->centerOn(QPointF(0, 0));
            const QPointF pivot = view->mapToScene(cursor);
            // The pivot is the platform's cursor (offscreen, as CTest runs this class, the process's own): say which of
            // the two conditions the zoom reads failed, if one does
            const QPoint expected = view->viewport()->mapToGlobal(cursor);
            QVERIFY2(QCursor::pos() == expected, qPrintable(QString("the system's cursor is at %1,%2, not at %3,%4")
                                                                .arg(QCursor::pos().x())
                                                                .arg(QCursor::pos().y())
                                                                .arg(expected.x())
                                                                .arg(expected.y())));
            QVERIFY2(view->underMouse(), "the view no longer counts the pointer as over it");
            view->zoomIn();
            const QPointF pivotNow(view->mapFromScene(pivot));
            QVERIFY2(QLineF(pivotNow, QPointF(cursor)).length() < 2.0,
                     qPrintable(QString("the zoom did not pivot on the pointer: the pivot is at %1,%2 px, not at %3,%4 px")
                                    .arg(pivotNow.x())
                                    .arg(pivotNow.y())
                                    .arg(cursor.x())
                                    .arg(cursor.y())));
        }

        for (const int delta : {120, -120}) {
            view->resetZoom();
            view->centerOn(QPointF(0, 0));
            const QPointF before = view->mapToScene(cursor);
            QWheelEvent wheel(QPointF(cursor), view->viewport()->mapToGlobal(cursor), QPoint(0, 0), QPoint(0, delta),
                              Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
            QApplication::sendEvent(view->viewport(), &wheel);

            // The scroll bars move in whole pixels, so the point can be off by a pixel in each direction.
            const QPointF now(view->mapFromScene(before));
            QVERIFY2(QLineF(now, QPointF(cursor)).length() < 2.0,
                     qPrintable(QString("pointer over %1, delta %2: the point under the cursor is at %3,%4 px, not at %5,%6 px")
                                    .arg(pointerOver)
                                    .arg(delta)
                                    .arg(now.x())
                                    .arg(now.y())
                                    .arg(cursor.x())
                                    .arg(cursor.y())));
        }
    }
}

void TestGraphicsView::testWheelNearSceneEdgeKeepsViewInsideSceneRect()
{
    // A scene rect tight around the circuit lets the view scroll only so far: from each corner of the viewport, a
    // wheel zoom in either direction leaves the visible area inside the scene rect.
    WorkSpace workspace;
    GraphicsView *view = workspace.view();
    const QRectF sceneRect(-400, -200, 1200, 1000);
    workspace.scene()->setSceneRect(sceneRect);
    layOutView(view);
    const QList<QPoint> corners{QPoint(20, 20), QPoint(780, 20), QPoint(20, 580), QPoint(780, 580)};

    for (const QPoint &cursor : corners) {
        for (const int delta : {120, -120}) {
            view->resetZoom();
            view->centerOn(sceneRect.center());
            QWheelEvent wheel(QPointF(cursor), view->viewport()->mapToGlobal(cursor), QPoint(0, 0), QPoint(0, delta),
                              Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
            QApplication::sendEvent(view->viewport(), &wheel);

            // Two scene units of slack: a scroll bar moves in whole pixels.
            const QRectF visible = view->mapToScene(view->viewport()->rect()).boundingRect();
            QVERIFY2(sceneRect.adjusted(-2, -2, 2, 2).contains(visible),
                     qPrintable(QString("cursor %1,%2 delta %3: the visible area %4,%5 %6x%7 leaves the scene rect")
                                    .arg(cursor.x())
                                    .arg(cursor.y())
                                    .arg(delta)
                                    .arg(visible.x())
                                    .arg(visible.y())
                                    .arg(visible.width())
                                    .arg(visible.height())));
        }
    }
}

void TestGraphicsView::testZoomKeepsItsPivotPastTheSceneRect()
{
    // A scene rect tight around the circuit, as a loaded file leaves it: zooming out shows more than it holds, and
    // each step, from each corner of the viewport, by wheel and by button with the pointer over the view, in either
    // layout direction, leaves the scene point under the cursor under it, as far out as the zoom goes and back in
    const QList<QPoint> corners{QPoint(20, 20), QPoint(780, 20), QPoint(20, 580), QPoint(780, 580)};
    for (const auto &[wheel, direction] : {std::pair(true, Qt::LeftToRight), std::pair(false, Qt::LeftToRight),
                                           std::pair(true, Qt::RightToLeft)}) {
        for (const QPoint &cursor : corners) {
            WorkSpace workspace;
            workspace.setLayoutDirection(direction);
            GraphicsView *view = workspace.view();
            const QRectF sceneRect(-400, -200, 1200, 1000);
            workspace.scene()->setSceneRect(sceneRect);
            layOutView(view);
            view->centerOn(sceneRect.center());
            if (!wheel) {
                view->setAttribute(Qt::WA_UnderMouse, true);
                QCursor::setPos(view->viewport()->mapToGlobal(cursor));
                QVERIFY2(QCursor::pos() == view->viewport()->mapToGlobal(cursor), "the platform did not place the cursor");
            }

            const auto step = [&](const int delta) {
                const QPointF before = view->mapToScene(cursor);
                if (wheel) {
                    QWheelEvent event(QPointF(cursor), view->viewport()->mapToGlobal(cursor), QPoint(0, 0),
                                      QPoint(0, delta), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
                    QApplication::sendEvent(view->viewport(), &event);
                } else if (delta > 0) {
                    view->zoomIn();
                } else {
                    view->zoomOut();
                }
                // The scroll bars move in whole pixels, so the point can be off by a pixel in each direction.
                const QPointF now(view->mapFromScene(before));
                return QLineF(now, QPointF(cursor)).length() < 2.0;
            };
            while (view->canZoomOut()) {
                QVERIFY2(step(-120), qPrintable(QString("wheel %1, direction %2, cursor %3,%4: zooming out to %5 moved the view")
                                                    .arg(wheel).arg(direction).arg(cursor.x()).arg(cursor.y())
                                                    .arg(view->transform().m11())));
            }
            while (view->canZoomIn()) {
                QVERIFY2(step(120), qPrintable(QString("wheel %1, direction %2, cursor %3,%4: zooming in to %5 moved the view")
                                                   .arg(wheel).arg(direction).arg(cursor.x()).arg(cursor.y())
                                                   .arg(view->transform().m11())));
            }
        }
    }
}

void TestGraphicsView::testZoomToFitIgnoresARubberBandOutsideTheCircuit()
{
    // Fitting after a rubber band dragged far from the circuit, then an edit, gives the zoom fitting gives without
    // the band: its item stays in the scene, hidden, with its last rectangle, and is not circuit
    const auto fittedZoom = [](bool dragBand) {
        WorkSpace workspace;
        auto *scene = workspace.scene();
        GraphicsView *view = workspace.view();
        view->resize(800, 600);
        auto *inputSwitch = new InputSwitch();
        scene->addItem(inputSwitch);
        if (dragBand) {
            TestUtils::ScenePointer pointer(scene);
            pointer.press(QPointF(3000, 3000));
            pointer.move(QPointF(3200, 3200));
            pointer.release(QPointF(3200, 3200));
        }
        auto *led = new Led();
        led->setPos(64, 0);
        scene->addItem(led);
        scene->setPropertyUpdateRequired();
        view->zoomToFit();
        return view->transform().m11();
    };

    QCOMPARE(fittedZoom(true), fittedZoom(false));
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
