/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
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

    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* NAND_H */
