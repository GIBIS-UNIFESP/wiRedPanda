// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class InputVcc : public GraphicElement
{
    Q_OBJECT

public:
    explicit InputVcc(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(InputVcc)
