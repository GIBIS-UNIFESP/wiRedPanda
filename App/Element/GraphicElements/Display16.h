// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/GraphicElement.h"

class Display16 : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit Display16(QGraphicsItem *parent = nullptr);

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
    QVector<QPixmap> bkg, a1, a2, b, c, d1, d2, e, f, g1, g2, h, j, k, l, m, n, dp;
    int m_colorNumber = 1;
};

Q_DECLARE_METATYPE(Display16)
