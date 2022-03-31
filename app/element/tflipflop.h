/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class TFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    explicit TFlipFlop(QGraphicsItem *parent = nullptr);
    ~TFlipFlop() override = default;
    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;

private:
    bool lastClk;
    signed char lastT;
    signed char lastQ;
};

