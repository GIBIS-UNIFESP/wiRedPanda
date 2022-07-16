/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Node : public GraphicElement
{
    Q_OBJECT

public:
    explicit Node(QGraphicsItem *parent = nullptr);

    void updatePortsProperties() override;
};

Q_DECLARE_METATYPE(Node)
