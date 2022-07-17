// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logictflipflop.h"

LogicTFlipFlop::LogicTFlipFlop()
    : LogicElement(4, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicTFlipFlop::_updateLogic(const QVector<bool> &inputs)
{
    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    bool T = inputs[0];
    bool clk = inputs[1];
    bool prst = inputs[2];
    bool clr = inputs[3];
    if (clk && !m_lastClk) {
        if (m_lastValue) {
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
    m_lastClk = clk;
    m_lastValue = T;
}
