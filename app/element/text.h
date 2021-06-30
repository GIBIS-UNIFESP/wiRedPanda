/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TEXT_H
#define TEXT_H

#include "graphicelement.h"

class Text : public GraphicElement
{
public:
    explicit Text(QGraphicsItem *parent = nullptr);
    ~Text() override = default;
};

#endif /* TEXT_H */
