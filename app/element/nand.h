// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class Nand : public GraphicElement
{
    Q_OBJECT

public:
    explicit Nand(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Nand)
