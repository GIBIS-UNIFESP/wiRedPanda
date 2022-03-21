/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef XOR_H
#define XOR_H

#include "graphicelement.h"

class Xor : public GraphicElement
{
public:
    explicit Xor(QGraphicsItem *parent = nullptr);
    ~Xor() override = default;

    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* XOR_H */
