/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class DFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    explicit DFlipFlop(QGraphicsItem *parent = nullptr);
    ~DFlipFlop() override = default;

    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;

private:
    bool lastClk;
    bool lastValue;
};

