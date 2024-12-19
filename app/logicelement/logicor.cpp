// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
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

    if (isTempSimulationOn()) {
        updateInputBuffer();
        const auto result = std::accumulate(m_inputBuffer.last().cbegin(), m_inputBuffer.last().cend(), false, std::bit_or<>());
        setOutputValue(result);
    } else {
        const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_or<>());
        setOutputValue(result);
    }
}
