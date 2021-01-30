/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NOR_H
#define NOR_H

#include "graphicelement.h"

class Nor : public GraphicElement
{
public:
    explicit Nor(QGraphicsItem *parent = nullptr);
    ~Nor() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return ElementType::NOR;
    }
    ElementGroup elementGroup() override
    {
        return ElementGroup::GATE;
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* NOR_H */
