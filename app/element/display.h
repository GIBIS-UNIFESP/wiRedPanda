// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class QNEPort;

class Display : public GraphicElement
{
    Q_OBJECT

public:
    explicit Display(QGraphicsItem *parent = nullptr);

    static QPixmap convertColor(const QImage &source, const bool red, const bool green, const bool blue);
    static void convertAllColors(QVector<QPixmap> &pixmaps);

    QString color() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setColor(const QString &color) override;
    void updatePortsProperties() override;

private:
    QString m_color = "Red";
    QVector<QPixmap> a, b, c, d, e, f, g, dp;
    int m_colorNumber = 1;
};

Q_DECLARE_METATYPE(Display)
