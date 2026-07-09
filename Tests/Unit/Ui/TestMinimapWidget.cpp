// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestMinimapWidget.h"

#include <QGraphicsRectItem>

#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MinimapWidget.h"

void TestMinimapWidget::testComputeTransformVerticalLetterbox()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 400, 100));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    // The minimap source is sceneRect ∪ itemsBoundingRect ∪ the visible viewport. Pin it with a
    // wide scene rect large enough to dominate the item and the (small, scale-1) viewport, so the
    // fitted aspect is deterministic: wide content → width is the limiting dimension.
    workspace.scene()->setSceneRect(QRectF(0, 0, 8000, 2000));

    MinimapWidget minimap(workspace.scene(), workspace.view());

    QRectF src;
    double scale = 0.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    QVERIFY(minimap.computeTransform(src, scale, dx, dy, usedW, usedH));

    // Ground truth computed exactly the way computeTransform() derives the source.
    QRectF expectedSrc = workspace.scene()->sceneRect().united(workspace.scene()->itemsBoundingRect());
    expectedSrc = expectedSrc.united(workspace.view()->mapToScene(workspace.view()->viewport()->rect()).boundingRect());
    const double expectedScale = qMin(220.0 / expectedSrc.width(), 160.0 / expectedSrc.height());
    const double expectedUsedW = expectedSrc.width() * expectedScale;
    const double expectedUsedH = expectedSrc.height() * expectedScale;

    QCOMPARE(src, expectedSrc);
    QCOMPARE(scale, expectedScale);
    QCOMPARE(usedW, expectedUsedW);
    QCOMPARE(usedH, expectedUsedH);
    QCOMPARE(dx, (220.0 - expectedUsedW) / 2.0);
    QCOMPARE(dy, (160.0 - expectedUsedH) / 2.0);

    // Wide content relative to the widget's aspect ratio: width is the fitted dimension.
    QCOMPARE(usedW, 220.0);
    QVERIFY(usedH < 160.0);
}

void TestMinimapWidget::testComputeTransformHorizontalLetterbox()
{
    WorkSpace workspace;
    auto *rectItem = new QGraphicsRectItem(QRectF(0, 0, 100, 400));
    rectItem->setPen(Qt::NoPen);
    workspace.scene()->addItem(rectItem);

    // Tall scene rect that dominates the item and viewport (see the vertical case): tall content
    // → height is the limiting dimension.
    workspace.scene()->setSceneRect(QRectF(0, 0, 2000, 8000));

    MinimapWidget minimap(workspace.scene(), workspace.view());

    QRectF src;
    double scale = 0.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    QVERIFY(minimap.computeTransform(src, scale, dx, dy, usedW, usedH));

    // Ground truth computed exactly the way computeTransform() derives the source.
    QRectF expectedSrc = workspace.scene()->sceneRect().united(workspace.scene()->itemsBoundingRect());
    expectedSrc = expectedSrc.united(workspace.view()->mapToScene(workspace.view()->viewport()->rect()).boundingRect());
    const double expectedScale = qMin(220.0 / expectedSrc.width(), 160.0 / expectedSrc.height());
    const double expectedUsedW = expectedSrc.width() * expectedScale;
    const double expectedUsedH = expectedSrc.height() * expectedScale;

    QCOMPARE(src, expectedSrc);
    QCOMPARE(scale, expectedScale);
    QCOMPARE(usedW, expectedUsedW);
    QCOMPARE(usedH, expectedUsedH);
    QCOMPARE(dx, (220.0 - expectedUsedW) / 2.0);
    QCOMPARE(dy, (160.0 - expectedUsedH) / 2.0);

    // Tall content relative to the widget's aspect ratio: height is the fitted dimension.
    QCOMPARE(usedH, 160.0);
    QVERIFY(usedW < 220.0);
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
