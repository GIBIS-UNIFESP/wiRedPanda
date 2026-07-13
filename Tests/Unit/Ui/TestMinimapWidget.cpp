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
