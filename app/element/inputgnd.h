// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class InputGnd : public GraphicElement
{
    Q_OBJECT

public:
    explicit InputGnd(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(InputGnd)
