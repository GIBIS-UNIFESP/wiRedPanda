// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicjkflipflop.h"

LogicJKFlipFlop::LogicJKFlipFlop()
    : LogicElement(5, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicJKFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool j = m_inputValues.at(0);
    const bool clk = m_inputValues.at(1);
    const bool k = m_inputValues.at(2);
    const bool prst = m_inputValues.at(3);
    const bool clr = m_inputValues.at(4);

    // Asynchronous preset/clear handling (active low inputs) - handle first
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
    } else if (clk && !m_lastClk) {
        // Clock edge only when preset/clear are inactive
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

    m_lastClk = clk;
    m_lastK = k;
    m_lastJ = j;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
