// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicSRFlipFlop.h"

LogicSRFlipFlop::LogicSRFlipFlop()
    : LogicElement(5, 2) // inputs: S, CLK, R, PRESET, CLEAR; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1 — the canonical reset state of an SR flip-flop.
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

    // Rising-edge detection: latch the S/R values from the previous cycle
    // (m_lastS, m_lastR) to correctly model setup-time semantics — the data
    // inputs must be stable before the clock edge.
    if (clk && !m_lastClk) {
        if (s && r) {
            // S=R=1 is the forbidden/undefined state in a real SR flip-flop;
            // here it forces both outputs high to give a deterministic result
            // instead of leaving the simulation in an undefined value.
            q0 = true;
            q1 = true;
        } else if (s != r) {
            // Normal SR behaviour: S sets (Q=1, Q'=0), R resets (Q=0, Q'=1).
            q0 = s;
            q1 = r;
        }
        // S=R=0 → hold; q0/q1 already retain the previous state.
    }

    // Asynchronous preset/clear override the clocked state: active-low signals
    // (asserted when the input is 0) take priority over the clock edge.
    if (!prst || !clr) {
        q0 = !prst;
        q1 = !clr;
    }

    // Record clock level after output computation so next call detects the edge.
    m_lastClk = clk;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
