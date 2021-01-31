/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INPUTSWITCH_H
#define INPUTSWITCH_H

#include "graphicelement.h"
#include "input.h"

class InputSwitch : public GraphicElement, public Input
{
public:
    explicit InputSwitch(QGraphicsItem *parent = nullptr);
    ~InputSwitch() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    bool on;
    /* QGraphicsItem interface */
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /* GraphicElement interface */
public:
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    bool getOn() const override;
    void setOn(bool value) override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* INPUTSWITCH_H */
