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
    const bool D = inputs.at(0);
    const bool clk = inputs.at(1);
    const bool prst = inputs.at(2);
    const bool clr = inputs.at(3);

    if (clk && !m_lastClk) {
        q0 = m_lastValue;
        q1 = !m_lastValue;
    }

    if ((!prst) || (!clr)) {
        q0 = !prst;
        q1 = !clr;
    }

    m_lastClk = clk;
    m_lastValue = D;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
