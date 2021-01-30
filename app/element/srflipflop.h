/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
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
public:
    ElementType elementType() override
    {
        return ElementType::SRFLIPFLOP;
    }
    ElementGroup elementGroup() override
    {
        return ElementGroup::MEMORY;
    }
    void updatePorts() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* SRFLIPFLOP_H */
