// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicNand.h"

LogicNand::LogicNand(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicNand::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // NAND: Active only when NOT all inputs are Active (inversion of AND).
    const bool allActive = std::all_of(inputs().cbegin(), inputs().cend(),
                                       [](Status s) { return s == Status::Active; });
    setOutputValue(allActive ? Status::Inactive : Status::Active);
}
