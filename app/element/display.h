/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "graphicelement.h"

class QNEPort;

class Display : public GraphicElement
{
public:
    explicit Display(QGraphicsItem *parent = nullptr);
    ~Display() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    void refresh() override;
    void updatePorts() override;
    QVector<QPixmap> bkg, a, b, c, d, e, f, g, dp;
    QString m_color;
    int m_color_number;

    /* QGraphicsItem interface */
public:
    void setColor(const QString &getColor) override;
    QString getColor() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /* GraphicElement interface */
public:
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void setSkin(bool defaultSkin, const QString &filename) override;

public:
    static QPixmap convertColor(const QImage &src, bool blue, bool green, bool red);
    static void convertAllColors(QVector<QPixmap> &maps);
};
#endif /* DISPLAY_H */
