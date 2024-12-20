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
        q0 = !S;
        q1 = !S;
    } else if (S) {
        q0 = !R;
        q1 = R;
    } else if (R) {
        q0 = S;
        q1 = !S;
    }

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
