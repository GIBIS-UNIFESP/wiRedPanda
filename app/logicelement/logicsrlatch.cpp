// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicsrlatch.h"

LogicSRLatch::LogicSRLatch()
    : LogicElement(2, 2)
{
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

    if (S && R) {
        // Forbidden state S=1, R=1: For educational purposes, implement reset-dominant behavior
        // (Reset has priority over Set, commonly used in real SR latches)
        q0 = false;  // Q = 0 (reset state)
        q1 = true;   // Q̄ = 1 (complementary)
    } else if (S && !R) {
        // Set state: S=1, R=0 -> Q=1, Q̄=0
        q0 = true;   // Q = 1
        q1 = false;  // Q̄ = 0
    } else if (!S && R) {
        // Reset state: S=0, R=1 -> Q=0, Q̄=1
        q0 = false;  // Q = 0
        q1 = true;   // Q̄ = 1
    }
    // Hold state: S=0, R=0 -> maintain current values (no else needed, q0 and q1 unchanged)

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
