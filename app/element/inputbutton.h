/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INPUTBUTTON_H
#define INPUTBUTTON_H

#include "graphicelement.h"
#include "input.h"

class InputButton : public GraphicElement, public Input
{
public:
    explicit InputButton(QGraphicsItem *parent = nullptr);
    ~InputButton() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.
    bool on;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

public:
    bool getOn() const override;
    void setOn(const bool value) override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* INPUTBUTTON_H */
