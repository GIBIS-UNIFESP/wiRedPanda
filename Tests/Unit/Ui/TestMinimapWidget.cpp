// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestMinimapWidget.h"

#include <QGraphicsRectItem>
#include <QSignalSpy>

#include "App/Core/ThemeManager.h"
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

void TestMinimapWidget::testSizeHint()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());
    QCOMPARE(minimap.sizeHint(), QSize(200, 150));
}

void TestMinimapWidget::testPaintEventNoOpWithoutSceneOrView()
{
    MinimapWidget minimap(nullptr, nullptr);
    // Must not crash despite a null event -- the guard returns before the event (or anything
    // else) is touched.
    minimap.paintEvent(nullptr);
}

void TestMinimapWidget::testCursorForResizeModeMapsEachModeToExpectedCursor()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());
    using Mode = MinimapWidget::ResizeMode;

    QCOMPARE(minimap.cursorForResizeMode(Mode::Top), Qt::SizeVerCursor);
    QCOMPARE(minimap.cursorForResizeMode(Mode::Bottom), Qt::SizeVerCursor);
    QCOMPARE(minimap.cursorForResizeMode(Mode::Left), Qt::SizeHorCursor);
    QCOMPARE(minimap.cursorForResizeMode(Mode::Right), Qt::SizeHorCursor);
    QCOMPARE(minimap.cursorForResizeMode(Mode::TopLeft), Qt::SizeFDiagCursor);
    QCOMPARE(minimap.cursorForResizeMode(Mode::BottomRight), Qt::SizeFDiagCursor);
    QCOMPARE(minimap.cursorForResizeMode(Mode::TopRight), Qt::SizeBDiagCursor);
    QCOMPARE(minimap.cursorForResizeMode(Mode::BottomLeft), Qt::SizeBDiagCursor);
    QCOMPARE(minimap.cursorForResizeMode(Mode::None), Qt::ArrowCursor);
}

void TestMinimapWidget::testMousePressGuardsNullSceneOrView()
{
    MinimapWidget minimap(nullptr, nullptr);
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    minimap.mousePressEvent(&event);

    // The guard returns before touching any state -- none of the drag/resize/move flags
    // must flip just because a press arrived with nothing to navigate.
    QVERIFY(!minimap.m_resizing);
    QVERIFY(!minimap.m_moving);
    QVERIFY(!minimap.m_dragging);
}

void TestMinimapWidget::testMousePressOnResizeHandleStartsResizing()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    minimap.mousePressEvent(&event);

    QVERIFY(minimap.m_resizing);
    QCOMPARE(minimap.m_resizeMode, MinimapWidget::ResizeMode::TopLeft);
    QVERIFY(!minimap.m_moving);
    QVERIFY(!minimap.m_dragging);
    QVERIFY(event.isAccepted());

    minimap.releaseMouse();
}

void TestMinimapWidget::testMousePressOnMoveHandleStartsMoving()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    // Top strip (moveHandleRect), away from any corner's resize zone.
    const QPointF pos(minimap.width() / 2.0, 15.0);
    QMouseEvent event(QEvent::MouseButtonPress, pos, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    minimap.mousePressEvent(&event);

    QVERIFY(minimap.m_moving);
    QVERIFY(!minimap.m_resizing);
    QVERIFY(event.isAccepted());

    minimap.releaseMouse();
}

void TestMinimapWidget::testMousePressElsewhereCentersViewAndStartsDragging()
{
    WorkSpace workspace;
    workspace.view()->resize(100, 100); // small but non-degenerate viewport to center within
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 4000, 2000));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);
    workspace.scene()->setSceneRect(QRectF(0, 0, 4000, 2000));

    MinimapWidget minimap(workspace.scene(), workspace.view());

    const QPointF centerBefore = workspace.view()->mapToScene(workspace.view()->viewport()->rect().center());

    const QPointF pressPos(20.0, minimap.height() / 2.0); // interior, off-center
    QMouseEvent event(QEvent::MouseButtonPress, pressPos, pressPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    minimap.mousePressEvent(&event);

    QVERIFY(minimap.m_dragging);
    QVERIFY(!minimap.m_resizing);
    QVERIFY(!minimap.m_moving);
    QVERIFY(event.isAccepted());

    const QPointF centerAfter = workspace.view()->mapToScene(workspace.view()->viewport()->rect().center());
    QVERIFY2(centerAfter != centerBefore, "mousePressEvent must recenter the view on the pressed point");
}

void TestMinimapWidget::testWheelEventSwallowsEvent()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QWheelEvent event(QPointF(10, 10), QPointF(10, 10), QPoint(0, 0), QPoint(0, 120),
                       Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    minimap.wheelEvent(&event);

    QVERIFY(event.isAccepted());
}

void TestMinimapWidget::testApplyResizeFallsBackToSquareAspectWithNoScene()
{
    MinimapWidget minimap(nullptr, nullptr);
    minimap.m_resizeMode = MinimapWidget::ResizeMode::Right;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    const QRect oldGeom = minimap.geometry();

    minimap.applyResize(QPoint(40, 0));

    // No scene to derive an aspect ratio from -> falls back to 1:1, so a horizontal-only
    // drag still updates the height to match the new (now-square) width.
    QCOMPARE(minimap.width(), oldGeom.width() + 40);
    QCOMPARE(minimap.height(), minimap.width());
}

void TestMinimapWidget::testApplyResizeDiagonalHorizontalDominant()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 200)); // 2:1 aspect
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_resizeMode = MinimapWidget::ResizeMode::BottomRight;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    const QRect oldGeom = minimap.geometry();

    minimap.applyResize(QPoint(40, 5)); // horizontal delta dominates

    QCOMPARE(minimap.width(), oldGeom.width() + 40);
    QVERIFY2(qAbs(static_cast<double>(minimap.width()) / minimap.height() - 2.0) < 0.05,
             "height must be derived from the locked 2:1 aspect ratio");
}

void TestMinimapWidget::testApplyResizeDiagonalVerticalDominant()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 200)); // 2:1 aspect
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_resizeMode = MinimapWidget::ResizeMode::BottomRight;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    const QRect oldGeom = minimap.geometry();

    minimap.applyResize(QPoint(5, 40)); // vertical delta dominates

    QCOMPARE(minimap.height(), oldGeom.height() + 40);
    QVERIFY2(qAbs(static_cast<double>(minimap.width()) / minimap.height() - 2.0) < 0.05,
             "width must be derived from the locked 2:1 aspect ratio");
}

void TestMinimapWidget::testApplyResizeAtWidthFloorRederivesHeightFromAspect()
{
    // Regression test for a real bug: applyResize() used to clamp width/height to their
    // minimums independently (qMax) *before* checking whether a clamp had actually happened,
    // so the "re-derive the other dimension from the locked aspect ratio" branches right below
    // were dead code -- hitting the width floor silently fell back to the raw minimumHeight()
    // instead of a height that still matches the locked aspect.
    WorkSpace workspace;
    // 1.3:1 aspect -- close to, but distinct from, the widget's own minimum-size ratio
    // (160x120 = 1.333:1), so a wrong fallback-to-raw-minimum is numerically distinguishable
    // from the correct aspect-derived value (123 vs 120).
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 130, 100));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_resizeMode = MinimapWidget::ResizeMode::Right;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    minimap.applyResize(QPoint(-1000, 0)); // drag drastically inward, past the width floor

    QCOMPARE(minimap.width(), minimap.minimumWidth());
    QCOMPARE(minimap.height(), 123); // qRound(160 / 1.3), not the raw minimumHeight() of 120
}

void TestMinimapWidget::testApplyResizeAtHeightFloorRederivesWidthFromAspect()
{
    // Symmetric regression test to testApplyResizeAtWidthFloorRederivesHeightFromAspect(),
    // for the height-floor branch.
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 200)); // 2:1 aspect
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view()); // starts at 220x160
    minimap.m_resizeMode = MinimapWidget::ResizeMode::Bottom;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    minimap.applyResize(QPoint(0, -50)); // drag the bottom edge up past the height floor

    QCOMPARE(minimap.height(), minimap.minimumHeight());
    QCOMPARE(minimap.width(), 240); // qRound(120 * 2.0), not the raw (unclamped) 220
}

void TestMinimapWidget::testMoveByIsNoOpWithoutParent()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view()); // no parent
    const QPoint posBefore = minimap.pos();

    minimap.moveBy(QPoint(50, 50));

    QCOMPARE(minimap.pos(), posBefore);
}

void TestMinimapWidget::testMouseMoveWhileResizingAppliesResizeAndInvalidatesCache()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 200));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_resizing = true;
    minimap.m_resizeMode = MinimapWidget::ResizeMode::Right;
    minimap.m_lastGlobalPos = QPoint(0, 0);
    minimap.m_cacheDirty = false;
    QVERIFY(!minimap.m_throttle.isActive());

    QMouseEvent event(QEvent::MouseMove, QPointF(40, 0), QPointF(40, 0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseMoveEvent(&event);

    QCOMPARE(minimap.width(), 260);
    QVERIFY2(minimap.m_throttle.isActive(), "invalidateCache() must have started the throttle timer");
    QVERIFY(event.isAccepted());
}

void TestMinimapWidget::testMouseMoveWhileMovingRepositionsWidget()
{
    WorkSpace workspace;
    QWidget parentWidget;
    parentWidget.resize(400, 300);
    MinimapWidget minimap(workspace.scene(), workspace.view(), &parentWidget);
    minimap.move(50, 50);

    minimap.m_moving = true;
    minimap.m_lastGlobalPos = QPoint(100, 100);

    QMouseEvent event(QEvent::MouseMove, QPointF(30, 30), QPointF(130, 130), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseMoveEvent(&event);

    QCOMPARE(minimap.pos(), QPoint(80, 80)); // moved by the (130,130)-(100,100) delta
    QCOMPARE(minimap.m_lastGlobalPos, QPoint(130, 130));
    QVERIFY(event.isAccepted());
}

void TestMinimapWidget::testMouseMoveUpdatesHoverStateWhenNotDragging()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    QMouseEvent event(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseMoveEvent(&event);

    QCOMPARE(minimap.m_hoverResizeMode, MinimapWidget::ResizeMode::TopLeft);
    QVERIFY(!event.isAccepted()); // not dragging -- falls through to the base-class handler
}

void TestMinimapWidget::testMouseMoveFallsBackToBaseClassWithoutSceneOrView()
{
    MinimapWidget minimap(nullptr, nullptr);
    minimap.m_dragging = true; // dragging, but with no scene/view to center on

    QMouseEvent event(QEvent::MouseMove, QPointF(10, 10), QPointF(10, 10), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseMoveEvent(&event);

    QVERIFY(!event.isAccepted());
}

void TestMinimapWidget::testMouseMoveWhileDraggingRecentersView()
{
    WorkSpace workspace;
    workspace.view()->resize(100, 100); // small but non-degenerate viewport to center within
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 4000, 2000));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);
    workspace.scene()->setSceneRect(QRectF(0, 0, 4000, 2000));

    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_dragging = true;

    const QPointF centerBefore = workspace.view()->mapToScene(workspace.view()->viewport()->rect().center());

    const QPointF pos(20.0, minimap.height() / 2.0);
    QMouseEvent event(QEvent::MouseMove, pos, pos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseMoveEvent(&event);

    const QPointF centerAfter = workspace.view()->mapToScene(workspace.view()->viewport()->rect().center());
    QVERIFY2(centerAfter != centerBefore, "dragging must recenter the view on the new position");
    QVERIFY(event.isAccepted());
}

void TestMinimapWidget::testMouseReleaseWhileResizingStopsAndEmitsGeometryChanged()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_resizing = true;
    minimap.m_resizeMode = MinimapWidget::ResizeMode::Right;

    QSignalSpy spy(&minimap, &MinimapWidget::geometryChangeFinished);
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseReleaseEvent(&event);

    QVERIFY(!minimap.m_resizing);
    QCOMPARE(minimap.m_resizeMode, MinimapWidget::ResizeMode::None);
    QCOMPARE(spy.count(), 1);
    QVERIFY(event.isAccepted());
}

void TestMinimapWidget::testMouseReleaseWhileMovingStopsAndEmitsGeometryChanged()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_moving = true;

    QSignalSpy spy(&minimap, &MinimapWidget::geometryChangeFinished);
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseReleaseEvent(&event);

    QVERIFY(!minimap.m_moving);
    QCOMPARE(spy.count(), 1);
    QVERIFY(event.isAccepted());
}

void TestMinimapWidget::testMouseReleaseWhileDraggingStopsDragging()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());
    minimap.m_dragging = true;

    QSignalSpy spy(&minimap, &MinimapWidget::geometryChangeFinished);
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseReleaseEvent(&event);

    QVERIFY(!minimap.m_dragging);
    QCOMPARE(spy.count(), 0); // plain click-to-navigate doesn't emit geometryChangeFinished
    QVERIFY(event.isAccepted());
}

void TestMinimapWidget::testMouseReleaseFallsBackToBaseClassWhenIdle()
{
    MinimapWidget minimap(nullptr, nullptr);
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    minimap.mouseReleaseEvent(&event);

    QVERIFY(!event.isAccepted());
}

void TestMinimapWidget::testThemeChangeInvalidatesCache()
{
    WorkSpace workspace;
    MinimapWidget minimap(workspace.scene(), workspace.view());

    // Simulate a cache that's already up to date, then a theme switch delivered the way
    // ThemeManager actually delivers it -- not by calling invalidateCache() directly, which
    // would trivially pass without proving the theme-change connection itself exists.
    minimap.m_cacheDirty = false;
    QVERIFY(!minimap.m_throttle.isActive());

    ThemeManager::instance().themeChanged();

    // invalidateCache() (like circuitHasChanged()) goes through the throttle timer rather
    // than flipping m_cacheDirty immediately; the timer being started confirms the slot ran.
    QVERIFY(minimap.m_throttle.isActive());
}
