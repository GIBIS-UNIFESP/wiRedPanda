
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#include "logictflipflop.h"

LogicTFlipFlop::LogicTFlipFlop()
    : LogicElement(4, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicTFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool T = m_inputValues.at(0);
    const bool clk = m_inputValues.at(1);
    const bool prst = m_inputValues.at(2);
    const bool clr = m_inputValues.at(3);

    if (clk && !m_lastClk) {
        if (m_lastValue) {
            q0 = !q0;
            q1 = !q0;
        }
    }

    if (!prst || !clr) {
        q0 = !prst;
        q1 = !clr;
    }

    m_lastClk = clk;
    m_lastValue = T;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
