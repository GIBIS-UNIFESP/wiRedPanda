/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef OR_H
#define OR_H

#include "graphicelement.h"

class Or : public GraphicElement
{
public:
    explicit Or(QGraphicsItem *parent = nullptr);
    ~Or() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return ElementType::OR;
    }
    ElementGroup elementGroup() override
    {
        return ElementGroup::GATE;
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* OR_H */
