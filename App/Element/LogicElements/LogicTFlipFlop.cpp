// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicTFlipFlop.h"

LogicTFlipFlop::LogicTFlipFlop()
    : LogicElement(4, 2) // inputs: T, CLK, PRESET, CLEAR; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1
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

    // Rising-edge detection: T is sampled one cycle early (m_lastValue) to model
    // setup time — the T input must be stable before the clock edge fires.
    if (clk && !m_lastClk) {
        if (m_lastValue) {
            // T=1 on previous cycle → toggle both complementary outputs.
            // q0 is flipped first; q1 is then derived from the *already-flipped*
            // q0, keeping Q and Q' complementary without a temporary variable.
            q0 = !q0;
            q1 = !q0;
        }
        // T=0 → hold
    }

    // Asynchronous preset/clear: active-low, take effect immediately.
    if (!prst || !clr) {
        q0 = !prst;
        q1 = !clr;
    }

    m_lastClk = clk;
    m_lastValue = T;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}

