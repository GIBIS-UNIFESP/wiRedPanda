// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class Display16 : public GraphicElement
{
    Q_OBJECT

public:
    explicit Display16(QGraphicsItem *parent = nullptr);

    QString color() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setColor(const QString &color) override;
    void updatePortsProperties() override;

private:
    QString m_color = "Red";
    QVector<QPixmap> bkg, a1, a2, b, c, d1, d2, e, f, g1, g2, h, j, k, l, m, n, dp;
    int m_colorNumber = 1;
};

Q_DECLARE_METATYPE(Display16)
