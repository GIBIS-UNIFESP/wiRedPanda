// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicor.h"

#include <functional>

LogicOr::LogicOr(const int inputSize)
    : LogicElement(inputSize, 1, inputSize)
{
}

void LogicOr::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (!isTempSimulationOn()) {
        const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_or<>());
        setOutputValue(result);
    }
    else {
        const auto result = std::accumulate(inputBuffer.last().cbegin(), inputBuffer.last().cend(), false, std::bit_or<>());
        setOutputValue(result);
        updateInputBuffer();
    }

}

