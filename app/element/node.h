// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

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
