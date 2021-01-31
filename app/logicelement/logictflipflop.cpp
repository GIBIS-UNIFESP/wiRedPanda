// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logictflipflop.h"

LogicTFlipFlop::LogicTFlipFlop()
    : LogicElement(4, 2)
    , lastClk(false)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicTFlipFlop::_updateLogic(const std::vector<bool> &inputs)
{
    bool q0 = getOutputValue(0);
    bool q1 = getOutputValue(1);
    bool T = inputs[0];
    bool clk = inputs[1];
    bool prst = inputs[2];
    bool clr = inputs[3];
    if (clk && !lastClk) {
        if (lastValue) {
            q0 = !q0;
            q1 = !q0;
        }
    }
    if ((!prst) || (!clr)) {
        q0 = !prst;
        q1 = !clr;
    }
    setOutputValue(0, q0);
    setOutputValue(1, q1);
    lastClk = clk;
    lastValue = T;
    /* Reference: https://en.wikipedia.org/wiki/Flip-flop_(electronics)#T_flip-flop */
}