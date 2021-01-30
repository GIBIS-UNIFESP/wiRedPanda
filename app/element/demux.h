/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef DEMUX_H
#define DEMUX_H

#include "graphicelement.h"

#include <QObject>

class Demux : public GraphicElement
{
public:
    explicit Demux(QGraphicsItem *parent = nullptr);
    ~Demux() override = default;

    ElementGroup elementGroup() override
    {
        return (ElementGroup::MUX);
    }

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::DEMUX);
    }
    void updatePorts() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* DEMUX_H */
