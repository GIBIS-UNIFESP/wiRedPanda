// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
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
