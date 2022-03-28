// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scene.h"

#include "graphicelement.h"
#include "qneconnection.h"

#include <QGraphicsView>
#include <QPainter>

// TODO: stop using QGraphicsView *
Scene::Scene(QObject *parent)
    : QGraphicsScene(parent)
{
}

Scene::Scene(const QRectF &sceneRect, QObject *parent)
    : QGraphicsScene(sceneRect, parent)
{
}

Scene::Scene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
    : QGraphicsScene(x, y, width, height, parent)
{
}

int Scene::gridSize() const
{
    return m_gridSize;
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    QGraphicsScene::drawBackground(painter, rect);
    painter->setPen(m_dots);
    qreal left = int(rect.left()) - (int(rect.left()) % m_gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % m_gridSize);
    QVector<QPointF> points;
    for (qreal x = left; x < rect.right(); x += m_gridSize) {
        for (qreal y = top; y < rect.bottom(); y += m_gridSize) {
            points.append(QPointF(x, y));
        }
    }
    painter->drawPoints(points.data(), points.size());
}

void Scene::setDots(const QPen &dots)
{
    m_dots = dots;
}

QVector<GraphicElement *> Scene::getVisibleElements()
{
    const auto gviews = views();
    auto *graphicsView = gviews.first();
    if (!graphicsView->isActiveWindow()) {
        graphicsView = gviews.last();
    }
    auto visibleRect = graphicsView->mapToScene(graphicsView->viewport()->geometry()).boundingRect();

    return getElements(visibleRect);
}

QVector<GraphicElement *> Scene::getElements()
{
    QVector<GraphicElement *> elements;
    for (auto *item : items()) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
            elements.append(elm);
        }
    }
    return elements;
}

QVector<GraphicElement *> Scene::getElements(const QRectF &rect)
{
    QVector<GraphicElement *> elements;
    for (auto *item : items(rect)) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
            elements.append(elm);
        }
    }
    return elements;
}

QVector<QNEConnection *> Scene::getConnections()
{
    QVector<QNEConnection *> conns;
    for (auto *item : items()) {
        auto *conn = dynamic_cast<QNEConnection *>(item);
        if (conn) {
            conns.append(conn);
        }
    }
    return conns;
}

QVector<GraphicElement *> Scene::selectedElements()
{
    QVector<GraphicElement *> elements;
    for (auto *item : selectedItems()) {
        if (item->type() == GraphicElement::Type) {
            auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
            if (elm) {
                elements.append(elm);
            }
        }
    }
    return elements;
}
