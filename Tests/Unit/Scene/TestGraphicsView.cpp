// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestGraphicsView.h"

#include <utility>

#include <QApplication>
#include <QGraphicsScene>
#include <QKeyEvent>
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
    // Each platform's style draws its own frame and scroll bars, which would take their widths from the viewport. With
    // neither, the viewport is 800x600 everywhere; the hidden scroll bars still hold the scroll position and its clamp.
    // A hidden widget defers its resize event to show(), and the view lays out its viewport on that event, so the test
    // sends it.
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->resize(800, 600);
    QResizeEvent resize(view->size(), QSize());
    QCoreApplication::sendEvent(view, &resize);
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

    // On master a wheel zoom does not keep the scene point under the cursor: scale() pivots on the viewport's centre,
    // then centerOn() brings the zoomed point under the cursor to the centre. The rows pin it, for plan 2's fix.
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
