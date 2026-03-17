// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicSRFlipFlop.h"

LogicSRFlipFlop::LogicSRFlipFlop()
    : LogicElement(5, 2) // inputs: S, CLK, R, PRESET, CLEAR; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1 — the canonical reset state of an SR flip-flop.
    setOutputValue(0, Status::Inactive);
    setOutputValue(1, Status::Active);
}

void LogicSRFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    Status q0 = outputs().at(0);
    Status q1 = outputs().at(1);
    const Status s = inputs().at(0);
    const Status clk = inputs().at(1);
    const Status r = inputs().at(2);
    const Status prst = inputs().at(3);
    const Status clr = inputs().at(4);

    // Rising-edge detection: use m_lastS/m_lastR to prevent cascade.
    if (clk == Status::Active && m_lastClk != Status::Active) {
        if (m_lastS == Status::Active && m_lastR == Status::Active) {
            q0 = Status::Active;
            q1 = Status::Active;
        } else if (m_lastS != m_lastR) {
            q0 = m_lastS;
            q1 = m_lastR;
        }
        // S=R=0 → hold; q0/q1 already retain the previous state.
    }

    // Asynchronous preset/clear: active-low.  Only trigger on Inactive.
    if (prst == Status::Inactive || clr == Status::Inactive) {
        q0 = (prst == Status::Inactive) ? Status::Active : Status::Inactive;
        q1 = (clr == Status::Inactive) ? Status::Active : Status::Inactive;
    }

    m_lastClk = clk;
    m_lastS = s;
    m_lastR = r;

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
