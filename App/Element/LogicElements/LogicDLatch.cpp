// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicDLatch.h"

LogicDLatch::LogicDLatch()
    : LogicElement(2, 2) // inputs: D, Enable; outputs: Q, Q'
{
    // Power-on default: Q=0, Q'=1
    setOutputValue(0, Status::Inactive);
    setOutputValue(1, Status::Active);
}

void LogicDLatch::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    Status q0 = outputs().at(0);
    Status q1 = outputs().at(1);
    const Status D = inputs().at(0);
    const Status enable = inputs().at(1);

    // Level-sensitive latch: transparent when Enable is high — outputs follow D
    // continuously.  When Enable is low the state is held (no change).
    if (enable == Status::Active) {
        q0 = D;
        q1 = (D == Status::Active) ? Status::Inactive : Status::Active;
    }

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
