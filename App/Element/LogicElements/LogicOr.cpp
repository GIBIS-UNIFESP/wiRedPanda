// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicOr.h"

LogicOr::LogicOr(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicOr::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // Output is Active when at least one input is Active.
    const bool result = std::any_of(inputs().cbegin(), inputs().cend(),
                                    [](Status s) { return s == Status::Active; });
    setOutputValue(result ? Status::Active : Status::Inactive);
}
