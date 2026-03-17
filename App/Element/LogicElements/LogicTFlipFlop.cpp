// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicTFlipFlop.h"

LogicTFlipFlop::LogicTFlipFlop()
    : LogicElement(4, 2) // inputs: T, CLK, PRESET, CLEAR; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1
    setOutputValue(0, Status::Inactive);
    setOutputValue(1, Status::Active);
}

void LogicTFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    Status q0 = outputs().at(0);
    Status q1 = outputs().at(1);
    const Status T = inputs().at(0);
    const Status clk = inputs().at(1);
    const Status prst = inputs().at(2);
    const Status clr = inputs().at(3);

    // Rising-edge detection: current T read directly.
    if (clk == Status::Active && m_lastClk != Status::Active) {
        if (T == Status::Active) {
            q0 = (q0 == Status::Active) ? Status::Inactive : Status::Active;
            q1 = (q1 == Status::Active) ? Status::Inactive : Status::Active;
        }
        // T=0 → hold
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
