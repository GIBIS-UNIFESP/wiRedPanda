// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicsrflipflop.h"

LogicSRFlipFlop::LogicSRFlipFlop()
    : LogicElement(5, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicSRFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool s = m_inputValues.at(0);
    const bool clk = m_inputValues.at(1);
    const bool r = m_inputValues.at(2);
    const bool prst = m_inputValues.at(3);
    const bool clr = m_inputValues.at(4);

    if (clk && !m_lastClk) {
        if (s && r) {
            // Forbidden state S=R=1: In real hardware this is unpredictable.
            // For educational purposes, we implement S-dominant behavior
            // (S has priority over R, commonly used in real SR flip-flops)
            q0 = true;
            q1 = false;
        } else if (s && !r) {
            // Set state: S=1, R=0 -> Q=1, Q̄=0
            q0 = true;
            q1 = false;
        } else if (!s && r) {
            // Reset state: S=0, R=1 -> Q=0, Q̄=1
            q0 = false;
            q1 = true;
        }
        // Hold state: S=0, R=0 -> Q unchanged, Q̄ unchanged
    }

    if (!prst || !clr) {
        q0 = !prst;
        q1 = !clr;
    }

    m_lastClk = clk;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
