/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NOT_H
#define NOT_H

#include "graphicelement.h"

class Not : public GraphicElement
{
public:
    explicit Not(QGraphicsItem *parent = nullptr);
    ~Not() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return ElementType::NOT;
    }
    ElementGroup elementGroup() override
    {
        return ElementGroup::GATE;
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* NOT_H */
