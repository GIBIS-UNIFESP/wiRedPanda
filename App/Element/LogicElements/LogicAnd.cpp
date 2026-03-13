// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicAnd.h"

LogicAnd::LogicAnd(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicAnd::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // Output is Active only when every input is Active.
    const bool result = std::all_of(inputs().cbegin(), inputs().cend(),
                                    [](Status s) { return s == Status::Active; });
    setOutputValue(result ? Status::Active : Status::Inactive);
}
