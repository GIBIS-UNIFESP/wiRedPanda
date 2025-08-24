// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicdflipflop.h"

LogicDFlipFlop::LogicDFlipFlop()
    : LogicElement(4, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicDFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool D = m_inputValues.at(0);
    const bool clk = m_inputValues.at(1);
    const bool prst = m_inputValues.at(2);
    const bool clr = m_inputValues.at(3);

    // FIXED: Use CURRENT D value on rising edge (proper edge-triggered behavior)
    if (clk && !m_lastClk) {
        q0 = D;        // Use current D value, not previous
        q1 = !D;       // Complement of current D value
    }

    // Asynchronous preset and clear (active low)
    if (!prst || !clr) {
        q0 = !prst;
        q1 = !clr;
    }

    m_lastClk = clk;
    // Note: m_lastValue no longer needed for edge-triggered behavior

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
