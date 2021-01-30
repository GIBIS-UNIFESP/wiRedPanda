/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef DLATCH_H
#define DLATCH_H

#include "graphicelement.h"

class DLatch : public GraphicElement
{
public:
    explicit DLatch(QGraphicsItem *parent = nullptr);
    ~DLatch() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return ElementType::DLATCH;
    }
    ElementGroup elementGroup() override
    {
        return ElementGroup::MEMORY;
    }
    void updatePorts() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* DLATCH_H */
