/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "graphicelement.h"

class Display : public GraphicElement
{
public:
    explicit Display(QGraphicsItem *parent = nullptr);
    ~Display() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    void refresh() override;
    void updatePorts() override;
    QPixmap bkg, a, b, c, d, e, f, g, dp;

    /* QGraphicsItem interface */
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /* GraphicElement interface */
public:
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};
#endif /* DISPLAY_H */
