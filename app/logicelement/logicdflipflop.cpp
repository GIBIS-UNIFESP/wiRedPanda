// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicdflipflop.h"

LogicDFlipFlop::LogicDFlipFlop()
    : LogicElement(4, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicDFlipFlop::_updateLogic(const QVector<bool> &inputs)
{
    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    bool D = inputs[0];
    bool clk = inputs[1];
    bool prst = inputs[2];
    bool clr = inputs[3];
    if (clk && !m_lastClk) {
        q0 = m_lastValue;
        q1 = !m_lastValue;
    }
    if ((!prst) || (!clr)) {
        q0 = !prst;
        q1 = !clr;
    }
    setOutputValue(0, q0);
    setOutputValue(1, q1);
    m_lastClk = clk;
    m_lastValue = D;
}
