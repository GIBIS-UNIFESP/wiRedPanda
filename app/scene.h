/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QGraphicsScene>

class GraphicElement;
class QNEConnection;

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Scene(QObject *parent = nullptr);
    explicit Scene(const QRectF &sceneRect, QObject *parent = nullptr);
    explicit Scene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);

    QVector<GraphicElement *> getElements();
    QVector<GraphicElement *> getElements(const QRectF &rect);
    QVector<GraphicElement *> getVisibleElements();
    QVector<GraphicElement *> selectedElements();
    QVector<QNEConnection *> getConnections();
    int gridSize() const;
    void setDots(const QPen &dots);

private:
    static constexpr int m_gridSize = 16;

    void drawBackground(QPainter *painter, const QRectF &rect) override;

    QPen m_dots;
};

