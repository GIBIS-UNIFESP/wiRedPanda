// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/GraphicElement.h"

class QNEPort;

class Display7 : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit Display7(QGraphicsItem *parent = nullptr);

    // --- Color utilities ---

    static QPixmap convertColor(const QImage &source, const bool red, const bool green, const bool blue);
    static void convertAllColors(QVector<QPixmap> &pixmaps);

    // --- Color State ---

    QString color() const override;
    void setColor(const QString &color) override;

    // --- Visual ---

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void updatePortsProperties() override;

    // --- Serialization ---

    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void save(QDataStream &stream) const override;

private:
    // --- Members ---

    QString m_color = "Red";
    QVector<QPixmap> a, b, c, d, e, f, g, dp;
    int m_colorNumber = 1;
};

Q_DECLARE_METATYPE(Display7)
