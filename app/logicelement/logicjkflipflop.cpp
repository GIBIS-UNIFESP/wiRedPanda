// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicjkflipflop.h"

LogicJKFlipFlop::LogicJKFlipFlop()
    : LogicElement(5, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicJKFlipFlop::_updateLogic(const QVector<bool> &inputs)
{
    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool j = inputs.at(0);
    const bool clk = inputs.at(1);
    const bool k = inputs.at(2);
    const bool prst = inputs.at(3);
    const bool clr = inputs.at(4);

    if (clk && !m_lastClk) {
        if (m_lastJ && m_lastK) {
            std::swap(q0, q1);
        } else if (m_lastJ) {
            q0 = true;
            q1 = false;
        } else if (m_lastK) {
            q0 = false;
            q1 = true;
        }
    }

    if ((!prst) || (!clr)) {
        q0 = !prst;
        q1 = !clr;
    }

    m_lastClk = clk;
    m_lastK = k;
    m_lastJ = j;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
