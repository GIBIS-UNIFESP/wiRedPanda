/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LINE_H
#define LINE_H

#include "graphicelement.h"

class Line : public GraphicElement
{
public:
    explicit Line(QGraphicsItem *parent = nullptr);
    ~Line() override = default;
};

#endif /* LINE_H */
