// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicJKFlipFlop.h"

LogicJKFlipFlop::LogicJKFlipFlop()
    : LogicElement(5, 2) // inputs: J, CLK, K, PRESET, CLEAR; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1
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

    // Rising-edge detection; inputs sampled from the previous cycle (setup time).
    if (clk && !m_lastClk) {
        if (m_lastJ && m_lastK) {
            // J=K=1 → Toggle: swap Q and Q' outputs
            std::swap(q0, q1);
        } else if (m_lastJ) {
            // J=1, K=0 → Set
            q0 = true;
            q1 = false;
        } else if (m_lastK) {
            // J=0, K=1 → Reset
            q0 = false;
            q1 = true;
        }
        // J=0, K=0 → Hold (no change)
    }

    // Asynchronous preset/clear: active-low, override clock behaviour.
    if (!prst || !clr) {
        q0 = !prst;
        q1 = !clr;
    }

    // Update history after computing new state so that the current cycle's
    // values become "previous cycle" for the next rising-edge evaluation.
    m_lastClk = clk;
    m_lastK = k;
    m_lastJ = j;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
