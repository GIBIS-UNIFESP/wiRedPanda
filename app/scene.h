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
    Scene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);

    int gridSize() const;
    QVector<GraphicElement *> getElements();
    QVector<GraphicElement *> getElements(const QRectF &rect);
    QVector<QNEConnection *> getConnections();
    QVector<GraphicElement *> selectedElements();

    void setDots(const QPen &dots);

    QVector<GraphicElement *> getVisibleElements();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    static constexpr int m_gridSize = 16;

private:
    QPen m_dots;
};

