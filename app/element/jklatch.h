/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef JKLATCH_H
#define JKLATCH_H

#include "graphicelement.h"

class JKLatch : public GraphicElement
{
public:
    explicit JKLatch(QGraphicsItem *parent = nullptr);
    ~JKLatch() override = default;

    /* GraphicElement interface */
public:
    void updatePorts() override;
};

#endif /* JKLATCH_H */
