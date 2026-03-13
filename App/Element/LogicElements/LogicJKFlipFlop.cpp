// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicJKFlipFlop.h"

LogicJKFlipFlop::LogicJKFlipFlop()
    : LogicElement(5, 2) // inputs: J, CLK, K, PRESET, CLEAR; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1
    setOutputValue(0, Status::Inactive);
    setOutputValue(1, Status::Active);
}

void LogicJKFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    Status q0 = outputs().at(0);
    Status q1 = outputs().at(1);
    const Status j = inputs().at(0);
    const Status clk = inputs().at(1);
    const Status k = inputs().at(2);
    const Status prst = inputs().at(3);
    const Status clr = inputs().at(4);

    // Rising-edge detection; inputs sampled from the previous cycle (setup time).
    if (clk == Status::Active && m_lastClk != Status::Active) {
        if (m_lastJ == Status::Active && m_lastK == Status::Active) {
            // J=K=1 → Toggle: swap Q and Q' outputs
            std::swap(q0, q1);
        } else if (m_lastJ == Status::Active) {
            // J=1, K=0 → Set
            q0 = Status::Active;
            q1 = Status::Inactive;
        } else if (m_lastK == Status::Active) {
            // J=0, K=1 → Reset
            q0 = Status::Inactive;
            q1 = Status::Active;
        }
        // J=0, K=0 → Hold (no change)
    }

    // Asynchronous preset/clear: active-low, override clock behaviour.
    if (prst != Status::Active || clr != Status::Active) {
        q0 = (prst != Status::Active) ? Status::Active : Status::Inactive;
        q1 = (clr != Status::Active) ? Status::Active : Status::Inactive;
    }

    // Update history after computing new state so that the current cycle's
    // values become "previous cycle" for the next rising-edge evaluation.
    m_lastClk = clk;
    m_lastK = k;
    m_lastJ = j;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
