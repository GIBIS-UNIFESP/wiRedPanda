/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class JKLatch : public GraphicElement
{
    Q_OBJECT

public:
    explicit JKLatch(QGraphicsItem *parent = nullptr);
    ~JKLatch() override = default;

    void updatePorts() override;
};

