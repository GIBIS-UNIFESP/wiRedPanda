// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicSRLatch.h"

LogicSRLatch::LogicSRLatch()
    : LogicElement(2, 2) // inputs: S, R; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1 (reset state)
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicSRLatch::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool S = m_inputValues.at(0);
    const bool R = m_inputValues.at(1);

    // Level-sensitive SR latch — outputs update continuously while inputs are held.
    if (S && R) {
        // Forbidden state (S=R=1): both outputs forced to 0 (NOR-latch behaviour).
        q0 = false;
        q1 = false;
    } else if (S) {
        // Set: Q→1, Q'→0.
        q0 = true;
        q1 = false;
    } else if (R) {
        // Reset: Q→0, Q'→1.
        q0 = false;
        q1 = true;
    }
    // S=R=0 → hold; q0/q1 already retain the latched state.

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
