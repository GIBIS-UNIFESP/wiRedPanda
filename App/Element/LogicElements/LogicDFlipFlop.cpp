// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicDFlipFlop.h"

LogicDFlipFlop::LogicDFlipFlop()
    : LogicElement(4, 2) // inputs: D, CLK, PRESET, CLEAR; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1
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

    // On the rising clock edge, capture the D value from the previous simulation
    // cycle (m_lastValue) to simulate setup-time semantics: D must be stable
    // before the clock edge for the value to be reliably latched.
    if (clk && !m_lastClk) {
        q0 = m_lastValue;
        q1 = !m_lastValue;
    }

    // Asynchronous preset/clear override the clocked state; active-low signals.
    if (!prst || !clr) {
        // When both are asserted simultaneously (S=R=1 in underlying NOR latch),
        // both outputs are forced high — the metastable/forbidden state is
        // represented explicitly rather than left undefined.
        q0 = !prst;
        q1 = !clr;
    }

    // Store current clock and data for next cycle's edge detection.
    m_lastClk = clk;
    m_lastValue = D;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
