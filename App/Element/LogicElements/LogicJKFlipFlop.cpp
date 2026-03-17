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

    // Rising-edge detection; current J/K read directly (event-driven engine
    // guarantees predecessors have committed before this element evaluates).
    if (clk == Status::Active && m_lastClk != Status::Active) {
        if (j == Status::Active && k == Status::Active) {
            std::swap(q0, q1);
        } else if (j == Status::Active) {
            q0 = Status::Active;
            q1 = Status::Inactive;
        } else if (k == Status::Active) {
            q0 = Status::Inactive;
            q1 = Status::Active;
        }
        // J=0, K=0 → Hold (no change)
    }

    // Asynchronous preset/clear: active-low.  Only trigger on Inactive.
    if (prst == Status::Inactive || clr == Status::Inactive) {
        q0 = (prst == Status::Inactive) ? Status::Active : Status::Inactive;
        q1 = (clr == Status::Inactive) ? Status::Active : Status::Inactive;
    }

    m_lastClk = clk;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
