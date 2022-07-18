// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicdlatch.h"

LogicDLatch::LogicDLatch()
    : LogicElement(2, 2)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicDLatch::_updateLogic(const QVector<bool> &inputs)
{
    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool D = inputs.at(0);
    const bool enable = inputs.at(1);

    if (enable) {
        q0 = D;
        q1 = !D;
    }

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
