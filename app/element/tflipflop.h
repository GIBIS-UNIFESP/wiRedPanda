/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TFLIPFLOP_H
#define TFLIPFLOP_H

#include "graphicelement.h"

class TFlipFlop : public GraphicElement
{
    bool lastClk;
    signed char lastT;
    signed char lastQ;

public:
    explicit TFlipFlop(QGraphicsItem *parent = nullptr);
    ~TFlipFlop() override = default;
    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* TFLIPFLOP_H */
