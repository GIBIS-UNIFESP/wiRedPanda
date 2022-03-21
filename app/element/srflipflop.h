/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SRFLIPFLOP_H
#define SRFLIPFLOP_H

#include "graphicelement.h"

class SRFlipFlop : public GraphicElement
{
    bool lastClk;

public:
    explicit SRFlipFlop(QGraphicsItem *parent = nullptr);
    ~SRFlipFlop() override = default;

    /* GraphicElement interface */
    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* SRFLIPFLOP_H */
