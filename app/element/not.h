// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class Not : public GraphicElement
{
    Q_OBJECT

public:
    explicit Not(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Not)
