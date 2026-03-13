// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicSRLatch.h"

LogicSRLatch::LogicSRLatch()
    : LogicElement(2, 2) // inputs: S, R; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1 (reset state)
    setOutputValue(0, Status::Inactive);
    setOutputValue(1, Status::Active);
}

void LogicSRLatch::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    Status q0 = outputs().at(0);
    Status q1 = outputs().at(1);
    const Status S = inputs().at(0);
    const Status R = inputs().at(1);

    // Level-sensitive SR latch — outputs update continuously while inputs are held.
    if (S == Status::Active && R == Status::Active) {
        // Forbidden state (S=R=1): both outputs forced to 0 (NOR-latch behaviour).
        q0 = Status::Inactive;
        q1 = Status::Inactive;
    } else if (S == Status::Active) {
        // Set: Q→1, Q'→0.
        q0 = Status::Active;
        q1 = Status::Inactive;
    } else if (R == Status::Active) {
        // Reset: Q→0, Q'→1.
        q0 = Status::Inactive;
        q1 = Status::Active;
    }
    // S=R=0 → hold; q0/q1 already retain the latched state.

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
