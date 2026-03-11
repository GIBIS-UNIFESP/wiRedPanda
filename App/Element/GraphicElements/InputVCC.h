// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/GraphicElement.h"

class InputVcc : public GraphicElement
{
    Q_OBJECT

public:
    explicit InputVcc(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(InputVcc)
