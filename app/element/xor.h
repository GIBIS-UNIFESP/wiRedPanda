// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class Xor : public GraphicElement
{
    Q_OBJECT

public:
    explicit Xor(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Xor)
