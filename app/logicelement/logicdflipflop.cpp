// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicdflipflop.h"

LogicDFlipFlop::LogicDFlipFlop()
    : LogicElement(4, 2)
    , lastClk(false)
    , lastValue(true)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicDFlipFlop::_updateLogic(const std::vector<bool> &inputs)
{
    bool q0 = getOutputValue(0);
    bool q1 = getOutputValue(1);
    bool D = inputs[0];
    bool clk = inputs[1];
    bool prst = inputs[2];
    bool clr = inputs[3];
    if (clk && !lastClk) {
        q0 = lastValue;
        q1 = !lastValue;
    }
    if ((!prst) || (!clr)) {
        q0 = !prst;
        q1 = !clr;
    }
    setOutputValue(0, q0);
    setOutputValue(1, q1);
    lastClk = clk;
    lastValue = D;
    /* Reference: https://en.wikipedia.org/wiki/Flip-flop_(electronics)#T_flip-flop */
}
