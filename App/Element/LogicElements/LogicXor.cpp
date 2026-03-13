// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicXor.h"

LogicXor::LogicXor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // XOR: odd-parity detector — Active when an odd number of inputs are Active.
    // This generalises the 2-input XOR definition consistently across all gate widths.
    const int activeCount = std::count_if(inputs().cbegin(), inputs().cend(),
                                          [](Status s) { return s == Status::Active; });
    setOutputValue((activeCount % 2 != 0) ? Status::Active : Status::Inactive);
}
