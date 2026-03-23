// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicAnd.h"

#include <functional>

LogicAnd::LogicAnd(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicAnd::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // Identity for AND is true; any false input short-circuits the fold to false.
    const auto result = std::accumulate(inputs().cbegin(), inputs().cend(), true, std::bit_and<>());
    setOutputValue(result);
}

