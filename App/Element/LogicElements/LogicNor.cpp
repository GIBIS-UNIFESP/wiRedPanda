// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicNor.h"

LogicNor::LogicNor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicNor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // NOR: Active only when no input is Active (inversion of OR).
    const bool anyActive = std::any_of(inputs().cbegin(), inputs().cend(),
                                       [](Status s) { return s == Status::Active; });
    setOutputValue(anyActive ? Status::Inactive : Status::Active);
}
