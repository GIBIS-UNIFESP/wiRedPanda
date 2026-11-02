// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestElementOutline.h"

#include <QCoreApplication>
#include <QFrame>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <QPainterPath>
#include <QResizeEvent>
#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/WirelessNode.h"
#include "App/Scene/ClipboardManager.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

// A named namespace: a unity build can put this file in one translation unit with other tests' helpers.
namespace ElementOutlineDetail {

/// Adds to \a scene a node at \a pos in \a mode.
Element *addNode(Scene *scene, const QPointF &pos, const WirelessMode mode)
{
    auto *node = ElementFactory::buildElement(ElementType::Node);
    scene->addItem(node);
    node->setPos(pos);
    WirelessNode::setMode(*node, mode);
    return node;
}

/// A point of \a node's indicator, in the scene, outside its outline (a wireless node's indicator reaches above and
/// beside it).
QPointF decorationOnly(const Element *node)
{
    const QRectF decoration = node->body().decoration;
    return node->mapToScene(QPointF(decoration.center().x(), decoration.top() + 2));
}

} // namespace ElementOutlineDetail

using namespace ElementOutlineDetail;

void TestElementOutline::testADecorationIsDrawnButIsNotTheOutline()
{
    WorkSpace workspace;
    const Element *plain = addNode(workspace.scene(), {0, 0}, WirelessMode::None);
    QVERIFY(plain->body().decoration.isNull());
    QCOMPARE(plain->boundingRect(), plain->outlineRect());

    for (const auto mode : {WirelessMode::Tx, WirelessMode::Rx}) {
        const Element *node = addNode(workspace.scene(), {0, 0}, mode);
        const QRectF decoration = node->body().decoration;
        QVERIFY(!decoration.isNull());
        QVERIFY(!node->outlineRect().contains(decoration));
        // The outline is a plain node's; the bounds Qt indexes and repaints hold the indicator too
        QCOMPARE(node->outlineRect(), plain->outlineRect());
        QCOMPARE(node->boundingRect(), node->outlineRect().united(decoration));
    }
}

void TestElementOutline::testAPressOnADecorationMissesTheElement()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    for (const auto mode : {WirelessMode::Tx, WirelessMode::Rx}) {
        Element *node = addNode(scene, {200, 200}, mode);
        const QPointF point = decorationOnly(node);
        QVERIFY(node->sceneBoundingRect().contains(point) && !node->sceneOutlineRect().contains(point));
        const QGraphicsScene *qtScene = scene;
        QVERIFY(!qtScene->items(point).contains(node));
        QVERIFY(qtScene->items(node->mapToScene(node->pixmapCenter())).contains(node));
        delete node;
    }
}

void TestElementOutline::testARubberBandOverADecorationAloneSelectsNothing()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *node = addNode(scene, {200, 200}, WirelessMode::Tx);

    QPainterPath band;
    band.addRect(QRectF(decorationOnly(node) - QPointF(1, 1), QSizeF(2, 2)));
    scene->setSelectionArea(band);
    QVERIFY(!node->isSelected());

    QPainterPath overBody;
    overBody.addRect(QRectF(node->mapToScene(node->pixmapCenter()) - QPointF(1, 1), QSizeF(2, 2)));
    scene->setSelectionArea(overBody);
    QVERIFY(node->isSelected());
}

void TestElementOutline::testAlignLeftUsesTheOutline()
{
    // An Rx node's indicator reaches 12 px left of its outline, more than the half grid a move snaps to: aligned by
    // their bounds, the plain node's outline would stick out to the left
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *plain = addNode(scene, {96, 0}, WirelessMode::None);
    Element *wireless = addNode(scene, {0, 96}, WirelessMode::Rx);
    plain->setSelected(true);
    wireless->setSelected(true);

    scene->alignLeft();

    QCOMPARE(plain->sceneOutlineRect().left(), wireless->sceneOutlineRect().left());
}

void TestElementOutline::testDistributeHorizontallyUsesTheOutline()
{
    // Equal gaps between the outlines put the middle node on the grid; counting the Rx node's indicator would move it
    // 6 px, past the half grid's midpoint
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *left = addNode(scene, {0, 0}, WirelessMode::None);
    Element *middle = addNode(scene, {96, 0}, WirelessMode::Rx);
    Element *right = addNode(scene, {320, 0}, WirelessMode::None);
    for (auto *node : {left, middle, right}) {
        node->setSelected(true);
    }

    scene->distributeHorizontally();

    const qreal before = middle->sceneOutlineRect().left() - left->sceneOutlineRect().right();
    const qreal after = right->sceneOutlineRect().left() - middle->sceneOutlineRect().right();
    QCOMPARE(before, after);
}

// Each placing below puts what the outline gives on a grid line other than what the bounds would give: the indicator's
// 6 px above a Tx node, or 12 px right of it, moves a node to another line of the half grid moves snap to.

void TestElementOutline::testAlignRightUsesTheOutline()
{
    // A Tx node's indicator reaches 12 px right of its outline
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *wireless = addNode(scene, {0, 0}, WirelessMode::Tx);
    Element *plain = addNode(scene, {200, 96}, WirelessMode::None);
    wireless->setSelected(true);
    plain->setSelected(true);

    scene->alignRight();

    QCOMPARE(wireless->sceneOutlineRect().right(), plain->sceneOutlineRect().right());
}

void TestElementOutline::testAlignTopUsesTheOutline()
{
    // A Tx node's indicator reaches 6 px above its outline
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *wireless = addNode(scene, {0, 8}, WirelessMode::Tx);
    Element *plain = addNode(scene, {96, 96}, WirelessMode::None);
    wireless->setSelected(true);
    plain->setSelected(true);

    scene->alignTop();

    QCOMPARE(plain->sceneOutlineRect().top(), wireless->sceneOutlineRect().top());
}

void TestElementOutline::testAlignHorizontalCenterUsesTheOutline()
{
    // A Tx node's indicator moves its bounds' centre 6 px right of its outline's
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *plain = addNode(scene, {0, 0}, WirelessMode::None);
    Element *wireless = addNode(scene, {8, 96}, WirelessMode::Tx);
    plain->setSelected(true);
    wireless->setSelected(true);

    scene->alignHorizontalCenter();

    QCOMPARE(plain->sceneOutlineRect().center().x(), wireless->sceneOutlineRect().center().x());
}

void TestElementOutline::testAlignVerticalCenterUsesTheOutline()
{
    // A Tx node's indicator moves its bounds' centre 3 px above its outline's
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *plain = addNode(scene, {0, 0}, WirelessMode::None);
    Element *wireless = addNode(scene, {96, 8}, WirelessMode::Tx);
    plain->setSelected(true);
    wireless->setSelected(true);

    scene->alignVerticalCenter();

    QCOMPARE(plain->sceneOutlineRect().center().y(), wireless->sceneOutlineRect().center().y());
}

void TestElementOutline::testDistributeVerticallyUsesTheOutline()
{
    // The middle node goes where equal gaps between the outlines put it, on the grid line nearest
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *top = addNode(scene, {0, 0}, WirelessMode::None);
    Element *middle = addNode(scene, {0, 48}, WirelessMode::Tx);
    Element *bottom = addNode(scene, {0, 194}, WirelessMode::None);
    for (auto *node : {top, middle, bottom}) {
        node->setSelected(true);
    }
    const qreal heights = top->sceneOutlineRect().height() + middle->sceneOutlineRect().height()
        + bottom->sceneOutlineRect().height();
    const qreal gap = (bottom->sceneOutlineRect().bottom() - top->sceneOutlineRect().top() - heights) / 2;
    const qreal outlineAbovePos = middle->sceneOutlineRect().top() - middle->pos().y();
    const qreal expected = Element::snapToGrid({0, top->sceneOutlineRect().bottom() + gap - outlineAbovePos}).y();

    scene->distributeVertically();

    QCOMPARE(middle->pos().y(), expected);
}

void TestElementOutline::testAlignBottomIsTheBoundsBottom()
{
    // No decoration reaches below its element (a node's indicator goes above it), so aligning the bottoms by the
    // outline or by the bounds is the same: the outline is used, as by every other alignment
    WorkSpace workspace;
    auto *scene = workspace.scene();
    for (const auto mode : {WirelessMode::Tx, WirelessMode::Rx}) {
        const Element *node = addNode(scene, {0, 0}, mode);
        QCOMPARE(node->sceneOutlineRect().bottom(), node->sceneBoundingRect().bottom());
    }
}

void TestElementOutline::testTheCloneDragPictureFramesTheOutline()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *node = addNode(scene, {96, 96}, WirelessMode::Tx);
    node->setSelected(true);

    const DragPreview::Picture picture = scene->clipboardManager()->selectionDragPicture(node->scenePos());

    const QRectF framed = node->sceneOutlineRect().adjusted(-8, -8, 8, 8);
    QCOMPARE(picture.topLeft, framed.topLeft());
    QCOMPARE(picture.pixmap.deviceIndependentSize(), framed.size());
}

void TestElementOutline::testTheLabelEditorStartsOnTheOutline()
{
    // A node with no label yet: the editor opens on the node itself, at its outline's corner
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Element *node = addNode(scene, {96, 96}, WirelessMode::Tx);

    QGraphicsSceneMouseEvent doubleClick(QEvent::GraphicsSceneMouseDoubleClick);
    doubleClick.setScenePos(node->mapToScene(node->pixmapCenter()));
    doubleClick.setButton(Qt::LeftButton);
    doubleClick.setButtons(Qt::LeftButton);
    QCoreApplication::sendEvent(scene, &doubleClick);

    QGraphicsProxyWidget *editor = nullptr;
    for (auto *item : scene->items()) {
        if (auto *proxy = qgraphicsitem_cast<QGraphicsProxyWidget *>(item)) {
            editor = proxy;
        }
    }
    QVERIFY(editor);
    QCOMPARE(editor->pos(), node->sceneOutlineRect().topLeft());
}

void TestElementOutline::testZoomToFitFramesTheOutline()
{
    WorkSpace workspace;
    auto *view = workspace.view();
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->resize(800, 600);
    QResizeEvent resize(view->size(), QSize());
    QCoreApplication::sendEvent(view, &resize);

    Element *node = addNode(workspace.scene(), {96, 96}, WirelessMode::Tx);
    node->setSelected(true);

    view->zoomToFit();

    const QPointF centre = view->mapToScene(view->viewport()->rect().center());
    const QPointF outlineCentre = node->sceneOutlineRect().center();
    QVERIFY2(QLineF(centre, outlineCentre).length() < 1.0,
             qPrintable(QString("centred on %1,%2, the outline's centre is %3,%4")
                            .arg(centre.x()).arg(centre.y()).arg(outlineCentre.x()).arg(outlineCentre.y())));
}
