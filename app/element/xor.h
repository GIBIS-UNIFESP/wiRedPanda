/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
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
