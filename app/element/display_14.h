/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Display14 : public GraphicElement
{
    Q_OBJECT

public:
    explicit Display14(QGraphicsItem *parent = nullptr);
    ~Display14() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void setSkin(bool defaultSkin, const QString &filename) override;

    void refresh() override;
    void updatePorts() override;

    void setColor(const QString &getColor) override;
    QString getColor() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QVector<QPixmap> bkg, a, b, c, d, e, f, g1, g2, h, j, k, l, m, n, dp;
    QString m_color;
    int m_color_number;
};
