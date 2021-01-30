/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AND_H
#define AND_H

#include "graphicelement.h"

class And : public GraphicElement
{
public:
    explicit And(QGraphicsItem *parent = nullptr);
    ~And() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::AND);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::GATE);
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* AND_H */
