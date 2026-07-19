// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickMinimap.h"

#include <QSignalSpy>

#include "App/Core/Enums.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickMinimap.h"

void TestQuickMinimap::testMinimapContentRectEmptyWithNoContent()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    // No setSize() call -- an unbound canvas (width()==0, height()==0) also has an empty
    // visibleWorldRect(), so with no elements either, the union stays invalid.
    QVERIFY(canvas.minimapContentRect(200, 200).isEmpty());
}

void TestQuickMinimap::testMinimapContentRectEmptyWithZeroTargetSize()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    QVERIFY(canvas.minimapContentRect(0, 200).isEmpty());
    QVERIFY(canvas.minimapContentRect(200, 0).isEmpty());
    QVERIFY(canvas.minimapContentRect(-10, 200).isEmpty());
}

void TestQuickMinimap::testMinimapContentRectMatchesTargetAspectForWideContent()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));

    auto *a = ElementFactory::buildElement(ElementType::And);
    a->setPos(0, 0);
    auto *b = ElementFactory::buildElement(ElementType::And);
    b->setPos(4000, 50);
    canvas.receiveCommand(new CanvasAddItemsCommand({a}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({b}, &canvas));

    const QRectF content = canvas.minimapContentRect(200, 100); // 2:1 target
    QVERIFY(!content.isEmpty());
    QCOMPARE(content.width() / content.height(), 2.0);
}

void TestQuickMinimap::testMinimapContentRectMatchesTargetAspectForTallContent()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));

    auto *a = ElementFactory::buildElement(ElementType::And);
    a->setPos(0, 0);
    auto *b = ElementFactory::buildElement(ElementType::And);
    b->setPos(50, 4000);
    canvas.receiveCommand(new CanvasAddItemsCommand({a}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({b}, &canvas));

    const QRectF content = canvas.minimapContentRect(100, 200); // 1:2 target
    QVERIFY(!content.isEmpty());
    QCOMPARE(content.width() / content.height(), 0.5);
}

void TestQuickMinimap::testMinimapContentRectNeverCropsElementBounds()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));

    auto *a = ElementFactory::buildElement(ElementType::And);
    a->setPos(0, 0);
    auto *b = ElementFactory::buildElement(ElementType::And);
    b->setPos(4000, 3000);
    canvas.receiveCommand(new CanvasAddItemsCommand({a}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({b}, &canvas));

    const QRectF elementBounds = canvas.elementsBoundingRect();
    const QRectF content = canvas.minimapContentRect(320, 180); // an arbitrary widescreen target

    QVERIFY2(content.contains(elementBounds),
             "the fitted content rect must never crop the real circuit's bounding box");
}

void TestQuickMinimap::testContentAspectRatioDefaultsToOneWithNoCanvas()
{
    QuickMinimap minimap;
    QCOMPARE(minimap.contentAspectRatio(), 1.0);
}

void TestQuickMinimap::testContentAspectRatioReflectsCircuitBounds()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    sw->setPos(0, 0);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(400, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas));

    QuickMinimap minimap;
    minimap.setCanvas(&canvas);

    QVERIFY2(minimap.contentAspectRatio() > 1.0,
             "a circuit spread wide relative to its own element height should report aspect > 1");
}

void TestQuickMinimap::testNavigateToPansCanvasToClickedContentCorner()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(1000, 1000);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    QuickMinimap minimap;
    minimap.setCanvas(&canvas);
    minimap.setMinimapWidth(200);
    minimap.setMinimapHeight(200);

    const QRectF content = canvas.minimapContentRect(200, 200);
    QVERIFY(!content.isEmpty());

    // Clicking minimap-local (0, 0) must center the canvas on the content rect's own top-left
    // world point -- navigateTo()'s own scale/clamp formula, verified directly.
    minimap.navigateTo(0, 0);
    const QPointF center = canvas.visibleWorldRect().center();
    QVERIFY(qAbs(center.x() - content.left()) < 1.0);
    QVERIFY(qAbs(center.y() - content.top()) < 1.0);
}

void TestQuickMinimap::testNavigateToClampsOutOfBoundsToContentRect()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(1000, 1000);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    QuickMinimap minimap;
    minimap.setCanvas(&canvas);
    minimap.setMinimapWidth(200);
    minimap.setMinimapHeight(200);

    const QRectF content = canvas.minimapContentRect(200, 200);
    QVERIFY(!content.isEmpty());

    // Wildly out-of-range local coordinates clamp to the content rect's bottom-right corner.
    minimap.navigateTo(100000, 100000);
    const QPointF center = canvas.visibleWorldRect().center();
    QVERIFY(qAbs(center.x() - content.right()) < 1.0);
    QVERIFY(qAbs(center.y() - content.bottom()) < 1.0);
}

void TestQuickMinimap::testNavigateToNoOpWithoutMinimapSize()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(1000, 1000);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    QuickMinimap minimap;
    minimap.setCanvas(&canvas);
    // Deliberately never set minimapWidth/minimapHeight (defaults to 0).

    const QRectF before = canvas.visibleWorldRect();
    minimap.navigateTo(50, 50); // must be a real no-op, not a crash
    QCOMPARE(canvas.visibleWorldRect(), before);
}

void TestQuickMinimap::testGeometryPersistsViaSettings()
{
    const QRect original = Settings::minimapGeometry();

    QuickMinimap minimap;
    minimap.commitGeometry(10, 20, 300, 250);
    QCOMPARE(minimap.initialGeometry(), QRect(10, 20, 300, 250));

    Settings::setMinimapGeometry(original);
}

void TestQuickMinimap::testRegenerateProducesImageAndViewportRectAfterThrottle()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(400, 400));
    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(100, 100);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    QuickMinimap minimap;
    QVERIFY(minimap.imageUrl().isEmpty());

    minimap.setCanvas(&canvas);
    minimap.setMinimapWidth(200);
    minimap.setMinimapHeight(200);

    QSignalSpy spy(&minimap, &QuickMinimap::refreshed);
    QVERIFY2(spy.wait(1000), "regenerate() should fire once the 200ms throttle elapses");

    QVERIFY(!minimap.imageUrl().isEmpty());
    QVERIFY(!minimap.viewportRect().isEmpty());
}
