
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class And : public GraphicElement
{
    Q_OBJECT

public:
    explicit And(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(And)
