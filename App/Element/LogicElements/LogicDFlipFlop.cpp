// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicDFlipFlop.h"

LogicDFlipFlop::LogicDFlipFlop()
    : LogicElement(4, 2) // inputs: D, CLK, PRESET, CLEAR; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1
    setOutputValue(0, Status::Inactive);
    setOutputValue(1, Status::Active);
}

void LogicDFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    Status q0 = outputs().at(0);
    Status q1 = outputs().at(1);
    const Status D = inputs().at(0);
    const Status clk = inputs().at(1);
    const Status prst = inputs().at(2);
    const Status clr = inputs().at(3);

    // On the rising clock edge, capture the D value from the previous simulation
    // cycle (m_lastValue) to simulate setup-time semantics: D must be stable
    // before the clock edge for the value to be reliably latched.
    if (clk == Status::Active && m_lastClk != Status::Active) {
        q0 = m_lastValue;
        q1 = (m_lastValue == Status::Active) ? Status::Inactive : Status::Active;
    }

    // Asynchronous preset/clear override the clocked state; active-low signals.
    if (prst != Status::Active || clr != Status::Active) {
        // When both are asserted simultaneously (S=R=1 in underlying NOR latch),
        // both outputs are forced high — the metastable/forbidden state is
        // represented explicitly rather than left undefined.
        q0 = (prst != Status::Active) ? Status::Active : Status::Inactive;
        q1 = (clr != Status::Active) ? Status::Active : Status::Inactive;
    }

    // Store current clock and data for next cycle's edge detection.
    m_lastClk = clk;
    m_lastValue = D;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
