// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class Or : public GraphicElement
{
    Q_OBJECT

public:
    explicit Or(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Or)
