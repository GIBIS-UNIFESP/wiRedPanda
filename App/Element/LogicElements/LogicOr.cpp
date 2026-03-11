// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicOr.h"

#include <functional>

LogicOr::LogicOr(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicOr::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // Identity for OR is false; any true input will propagate through the fold.
    const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_or<>());
    setOutputValue(result);
}
