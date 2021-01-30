/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NAND_H
#define NAND_H

#include "graphicelement.h"

class Nand : public GraphicElement
{
public:
    explicit Nand(QGraphicsItem *parent = nullptr);
    ~Nand() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::NAND);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::GATE);
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* NAND_H */
