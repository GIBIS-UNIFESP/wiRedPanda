// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestMinimapWidget.h"

#include <QGraphicsRectItem>

#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MinimapWidget.h"

void TestMinimapWidget::testComputeTransformWideSourceFillsWidget()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 100));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    // The minimap source is sceneRect ∪ itemsBoundingRect ∪ the visible viewport. Pin it with a
    // wide scene rect large enough to dominate the item and the (small, scale-1) viewport, so the
    // expansion branch under test (source wider than the widget → grow height) is deterministic.
    workspace.scene()->setSceneRect(QRectF(0, 0, 8000, 2000));

    MinimapWidget minimap(workspace.scene(), workspace.view());

    QRectF src;
    double scale = 0.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    QVERIFY(minimap.computeTransform(src, scale, dx, dy, usedW, usedH));

    // Pre-expansion union, derived the same way computeTransform() builds it.
    QRectF unionSrc = workspace.scene()->sceneRect().united(workspace.scene()->itemsBoundingRect());
    unionSrc = unionSrc.united(workspace.view()->mapToScene(workspace.view()->viewport()->rect()).boundingRect());

    // The source is expanded to the widget's aspect ratio so the scene fills the whole widget:
    // no letterbox offsets, used size equals the widget size, and the source still contains the
    // original (un-expanded) union.
    QVERIFY(qFuzzyIsNull(dx));
    QVERIFY(qFuzzyIsNull(dy));
    QVERIFY(qAbs(usedW - 220.0) < 1e-6);
    QVERIFY(qAbs(usedH - 160.0) < 1e-6);
    QVERIFY(qAbs(src.width() / src.height() - 220.0 / 160.0) < 1e-6);
    QVERIFY(src.contains(unionSrc));
    // Wide source → grow height, width unchanged.
    QVERIFY(qFuzzyCompare(src.width(), unionSrc.width()));
    QVERIFY(src.height() > unionSrc.height());
}

void TestMinimapWidget::testComputeTransformTallSourceFillsWidget()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 100, 400));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    // Tall scene rect that dominates the item and viewport (see the wide case): exercises the
    // other expansion branch (source taller than the widget → grow width).
    workspace.scene()->setSceneRect(QRectF(0, 0, 2000, 8000));

    MinimapWidget minimap(workspace.scene(), workspace.view());

    QRectF src;
    double scale = 0.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    QVERIFY(minimap.computeTransform(src, scale, dx, dy, usedW, usedH));

    QRectF unionSrc = workspace.scene()->sceneRect().united(workspace.scene()->itemsBoundingRect());
    unionSrc = unionSrc.united(workspace.view()->mapToScene(workspace.view()->viewport()->rect()).boundingRect());

    QVERIFY(qFuzzyIsNull(dx));
    QVERIFY(qFuzzyIsNull(dy));
    QVERIFY(qAbs(usedW - 220.0) < 1e-6);
    QVERIFY(qAbs(usedH - 160.0) < 1e-6);
    QVERIFY(qAbs(src.width() / src.height() - 220.0 / 160.0) < 1e-6);
    QVERIFY(src.contains(unionSrc));
    // Tall source → grow width, height unchanged.
    QVERIFY(qFuzzyCompare(src.height(), unionSrc.height()));
    QVERIFY(src.width() > unionSrc.width());
}

void TestMinimapWidget::testComputeTransformNullScene()
{
    // computeTransform's own `if (!m_scene) return false;` guard -- the one early-return
    // path actually reachable in practice, since a live Scene always has a valid (if tiny)
    // itemsBoundingRect() from its permanent selection-rect item.
    MinimapWidget minimap(nullptr, nullptr);

    QRectF src;
    double scale = 0.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    QVERIFY(!minimap.computeTransform(src, scale, dx, dy, usedW, usedH));
}

void TestMinimapWidget::testWidgetToSceneTopLeft()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 100));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QRectF src;
    double scale = 0.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    QVERIFY(minimap.computeTransform(src, scale, dx, dy, usedW, usedH));

    QCOMPARE(minimap.widgetToScene(QPointF(dx, dy)), src.topLeft());
}

void TestMinimapWidget::testWidgetToSceneCenter()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 100));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QRectF src;
    double scale = 0.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    QVERIFY(minimap.computeTransform(src, scale, dx, dy, usedW, usedH));

    const QPointF widgetCenter(dx + usedW / 2.0, dy + usedH / 2.0);
    QCOMPARE(minimap.widgetToScene(widgetCenter), src.center());
}

void TestMinimapWidget::testWidgetToSceneClampsOutOfBoundsX()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 100));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QRectF src;
    double scale = 0.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    QVERIFY(minimap.computeTransform(src, scale, dx, dy, usedW, usedH));

    const double midY = dy + usedH / 2.0;
    QCOMPARE(minimap.widgetToScene(QPointF(-50.0, midY)).x(), src.left());
    QCOMPARE(minimap.widgetToScene(QPointF(1000.0, midY)).x(), src.right());
}

void TestMinimapWidget::testWidgetToSceneNullSceneDegradesToOrigin()
{
    MinimapWidget minimap(nullptr, nullptr);
    QCOMPARE(minimap.widgetToScene(QPointF(10.0, 10.0)), QPointF());
}

void TestMinimapWidget::testAccessibleNameSet()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());
    QVERIFY(!minimap.accessibleName().isEmpty());
    QVERIFY(!minimap.whatsThis().isEmpty());
}

void TestMinimapWidget::testResizeModeAtEdgesAndCorners()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view()); // default size 220x160

    using Mode = MinimapWidget::ResizeMode;
    const int w = minimap.width();
    const int h = minimap.height();

    QCOMPARE(minimap.resizeModeAt(QPoint(0, h / 2)), Mode::Left);
    QCOMPARE(minimap.resizeModeAt(QPoint(w - 1, h / 2)), Mode::Right);
    QCOMPARE(minimap.resizeModeAt(QPoint(w / 2, 0)), Mode::Top);
    QCOMPARE(minimap.resizeModeAt(QPoint(w / 2, h - 1)), Mode::Bottom);
    QCOMPARE(minimap.resizeModeAt(QPoint(0, 0)), Mode::TopLeft);
    QCOMPARE(minimap.resizeModeAt(QPoint(w - 1, 0)), Mode::TopRight);
    QCOMPARE(minimap.resizeModeAt(QPoint(0, h - 1)), Mode::BottomLeft);
    QCOMPARE(minimap.resizeModeAt(QPoint(w - 1, h - 1)), Mode::BottomRight);
}

void TestMinimapWidget::testResizeModeAtInteriorIsNone()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QCOMPARE(minimap.resizeModeAt(QPoint(minimap.width() / 2, minimap.height() / 2)), MinimapWidget::ResizeMode::None);
}

void TestMinimapWidget::testApplyResizePreservesAspectRatio()
{
    WorkSpace workspace;
    // A 2:1 items bounding rect drives applyResize()'s locked aspect ratio.
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 200));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view()); // starts at 220x160
    minimap.m_resizeMode = MinimapWidget::ResizeMode::Right;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    minimap.applyResize(QPoint(40, 0)); // drag the right edge out by 40px

    QCOMPARE(minimap.width(), 260);
    QVERIFY2(qAbs(static_cast<double>(minimap.width()) / minimap.height() - 2.0) < 0.05,
             "resize must preserve the scene's 2:1 aspect ratio");
}

void TestMinimapWidget::testApplyResizeClampsToMinimumSize()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 200));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_resizeMode = MinimapWidget::ResizeMode::Right;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    minimap.applyResize(QPoint(-1000, 0)); // drag drastically inward, past the minimum

    QCOMPARE(minimap.width(), minimap.minimumWidth());
    QCOMPARE(minimap.height(), minimap.minimumHeight());
}

void TestMinimapWidget::testApplyResizeFromLeftKeepsRightEdgeFixed()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 200));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view());
    const QRect oldGeom = minimap.geometry();

    minimap.m_resizeMode = MinimapWidget::ResizeMode::Left;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    minimap.applyResize(QPoint(-40, 0)); // drag the left edge outward by 40px

    // Regression: applyResize() used to combine setSize() (which keeps the top-left corner
    // fixed while recomputing bottom-right) with a subsequent setTop()/setLeft() anchored on
    // that already-shifted rect -- doubling the resize delta and drifting the anchor (right)
    // edge instead of holding it fixed.
    QCOMPARE(minimap.geometry().right(), oldGeom.right());
    QVERIFY2(minimap.width() > oldGeom.width(), "dragging the left edge outward must grow the widget");
}

void TestMinimapWidget::testApplyResizeFromTopKeepsBottomEdgeFixed()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 200));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view());
    const QRect oldGeom = minimap.geometry();

    minimap.m_resizeMode = MinimapWidget::ResizeMode::Top;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    minimap.applyResize(QPoint(0, -40)); // drag the top edge outward by 40px

    // Same regression as testApplyResizeFromLeftKeepsRightEdgeFixed(), on the vertical axis.
    QCOMPARE(minimap.geometry().bottom(), oldGeom.bottom());
    QVERIFY2(minimap.height() > oldGeom.height(), "dragging the top edge outward must grow the widget");
}

void TestMinimapWidget::testMoveHandleRectCoversTopStrip()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QCOMPARE(minimap.moveHandleRect(), QRect(0, 0, minimap.width(), 24));
}

void TestMinimapWidget::testIsMoveHandleDetectsTopStripOnly()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QVERIFY(minimap.isMoveHandle(QPoint(minimap.width() / 2, 5)));
    QVERIFY(!minimap.isMoveHandle(QPoint(minimap.width() / 2, 50)));
}

void TestMinimapWidget::testMoveByClampsToParentBounds()
{
    WorkSpace workspace;
    QWidget parentWidget;
    parentWidget.resize(300, 200);

    MinimapWidget minimap(workspace.scene(), workspace.view(), &parentWidget);
    minimap.move(50, 50);

    // A huge move-right/down delta must clamp to the 12px margin, not push the widget
    // (partially or fully) outside the parent.
    minimap.moveBy(QPoint(10000, 10000));

    const int margin = 12;
    QCOMPARE(minimap.x(), parentWidget.width() - minimap.width() - margin);
    QCOMPARE(minimap.y(), parentWidget.height() - minimap.height() - margin);

    // Same for the opposite direction.
    minimap.moveBy(QPoint(-10000, -10000));
    QCOMPARE(minimap.x(), margin);
    QCOMPARE(minimap.y(), margin);
}

void TestMinimapWidget::testHoverStateOverCornerSetsResizeCursorAndHighlight()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    minimap.updateHoverState(QPoint(0, 0)); // top-left corner

    QCOMPARE(minimap.m_hoverResizeMode, MinimapWidget::ResizeMode::TopLeft);
    QVERIFY(!minimap.m_hoverMoveHandle);
    QCOMPARE(minimap.cursor().shape(), Qt::SizeFDiagCursor);
}

void TestMinimapWidget::testHoverStateOverMoveStripSetsOpenHandCursorAndHighlight()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    minimap.updateHoverState(QPoint(minimap.width() / 2, 15)); // top strip, below the top resize zone (y<=8)

    QCOMPARE(minimap.m_hoverResizeMode, MinimapWidget::ResizeMode::None);
    QVERIFY(minimap.m_hoverMoveHandle);
    QCOMPARE(minimap.cursor().shape(), Qt::OpenHandCursor);
}

void TestMinimapWidget::testHoverStateOverInteriorClearsHighlightAndCursor()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    minimap.updateHoverState(QPoint(0, 0)); // corner first, so state actually changes back below
    minimap.updateHoverState(QPoint(minimap.width() / 2, minimap.height() / 2)); // interior

    QCOMPARE(minimap.m_hoverResizeMode, MinimapWidget::ResizeMode::None);
    QVERIFY(!minimap.m_hoverMoveHandle);
    QCOMPARE(minimap.cursor().shape(), Qt::ArrowCursor);
}
