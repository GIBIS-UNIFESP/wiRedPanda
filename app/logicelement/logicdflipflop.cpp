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

    if (clk && !m_lastClk) {
        q0 = m_lastValue;
        q1 = !m_lastValue;
    }

    // Asynchronous preset/clear handling (active low inputs)
    if (!prst && !clr) {
        // Both preset and clear active: Clear has priority (reset-dominant)
        q0 = false;  // Q = 0 (clear state)
        q1 = true;   // Q̄ = 1 (complementary)
    } else if (!prst) {
        // Only preset active: Set Q=1, Q̄=0
        q0 = true;   // Q = 1 (preset state)
        q1 = false;  // Q̄ = 0 (complementary)
    } else if (!clr) {
        // Only clear active: Set Q=0, Q̄=1
        q0 = false;  // Q = 0 (clear state)
        q1 = true;   // Q̄ = 1 (complementary)
    }

    m_lastClk = clk;
    m_lastValue = D;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
