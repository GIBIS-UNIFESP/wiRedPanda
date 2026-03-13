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

    // Rising-edge detection: T is sampled one cycle early (m_lastValue) to model
    // setup time — the T input must be stable before the clock edge fires.
    if (clk == Status::Active && m_lastClk != Status::Active) {
        if (m_lastValue == Status::Active) {
            // T=1 on previous cycle → toggle both complementary outputs.
            q0 = (q0 == Status::Active) ? Status::Inactive : Status::Active;
            q1 = (q1 == Status::Active) ? Status::Inactive : Status::Active;
        }
        // T=0 → hold
    }

    // Asynchronous preset/clear: active-low, take effect immediately.
    if (prst != Status::Active || clr != Status::Active) {
        q0 = (prst != Status::Active) ? Status::Active : Status::Inactive;
        q1 = (clr != Status::Active) ? Status::Active : Status::Inactive;
    }

    m_lastClk = clk;
    m_lastValue = T;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
