// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicsrflipflop.h"

LogicSRFlipFlop::LogicSRFlipFlop()
    : LogicElement(5, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicSRFlipFlop::_updateLogic(const QVector<bool> &inputs)
{
    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool s = inputs.at(0);
    const bool clk = inputs.at(1);
    const bool r = inputs.at(2);
    const bool prst = inputs.at(3);
    const bool clr = inputs.at(4);

    if (clk && !m_lastClk) {
        if (s && r) {
            q0 = true;
            q1 = true;
        } else if (s != r) {
            q0 = s;
            q1 = r;
        }
    }

    if ((!prst) || (!clr)) {
        q0 = !prst;
        q1 = !clr;
    }

    m_lastClk = clk;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
