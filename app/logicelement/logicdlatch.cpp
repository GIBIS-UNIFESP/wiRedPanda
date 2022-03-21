// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicdlatch.h"

LogicDLatch::LogicDLatch()
    : LogicElement(2, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicDLatch::_updateLogic(const std::vector<bool> &inputs)
{
    bool q0 = getOutputValue(0);
    bool q1 = getOutputValue(1);
    bool D = inputs[0];
    bool enable = inputs[1];
    if (enable) {
        q0 = D;
        q1 = !D;
    }
    setOutputValue(0, q0);
    setOutputValue(1, q1);
}