// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFont>
#include <QGraphicsObject>
#include <QStringList>

class ElementWarningLabel : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit ElementWarningLabel(const QStringList &lines, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QStringList m_lines;
    QFont m_font;
    QRectF m_bounds;
};
