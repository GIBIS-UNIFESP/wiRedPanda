
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class Line : public GraphicElement
{
    Q_OBJECT

public:
    explicit Line(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Line)
