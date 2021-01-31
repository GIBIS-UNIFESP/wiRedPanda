/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef DFLIPFLOP_H
#define DFLIPFLOP_H

#include "graphicelement.h"

class DFlipFlop : public GraphicElement
{
    bool lastClk;
    bool lastValue;

public:
    explicit DFlipFlop(QGraphicsItem *parent = nullptr);
    ~DFlipFlop() override = default;

    void updatePorts() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif // DFLIPFLOP_H
