/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "logicelement.h"

class LogicDFlipFlop : public LogicElement
{
public:
    explicit LogicDFlipFlop();

protected:
    void _updateLogic(const std::vector<bool> &inputs) override;

private:
    bool lastClk;
    bool lastValue;
};

