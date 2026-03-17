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

    // On the rising clock edge, capture m_lastValue (D from before the current
    // event batch).  In the blocking FIFO engine, multiple flip-flops sharing
    // the same clock are queued together.  Without m_lastValue, each flip-flop
    // would see the preceding flip-flop's already-updated Q, causing shift
    // registers to cascade all stages in one clock cycle.
    if (clk == Status::Active && m_lastClk != Status::Active) {
        q0 = m_lastValue;
        q1 = (m_lastValue == Status::Active) ? Status::Inactive : Status::Active;
    }

    // Asynchronous preset/clear: active-low.  Only trigger on specifically
    // asserted (Inactive); Unknown/Error must not falsely activate the override.
    if (prst == Status::Inactive || clr == Status::Inactive) {
        q0 = (prst == Status::Inactive) ? Status::Active : Status::Inactive;
        q1 = (clr == Status::Inactive) ? Status::Active : Status::Inactive;
    }

    m_lastClk = clk;
    m_lastValue = D;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
