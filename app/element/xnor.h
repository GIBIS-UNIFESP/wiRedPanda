/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Xnor : public GraphicElement
{
    Q_OBJECT

public:
    explicit Xnor(QGraphicsItem *parent = nullptr);
    ~Xnor() override = default;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

