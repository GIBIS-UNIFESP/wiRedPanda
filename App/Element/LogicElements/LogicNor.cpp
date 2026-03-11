// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicNor.h"

#include <functional>

LogicNor::LogicNor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicNor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // Reuse the OR fold (identity=false) and invert at the end, mirroring LogicOr.
    const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_or<>());
    setOutputValue(!result);
}

