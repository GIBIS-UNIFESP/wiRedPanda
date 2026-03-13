// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicXnor.h"

LogicXnor::LogicXnor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXnor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // XNOR: even-parity detector — Active when an even number of inputs are Active.
    // This is the standard n-input XNOR convention used in this simulator.
    const int activeCount = std::count_if(inputs().cbegin(), inputs().cend(),
                                          [](Status s) { return s == Status::Active; });
    setOutputValue((activeCount % 2 == 0) ? Status::Active : Status::Inactive);
}
