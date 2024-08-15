// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnor.h"

#include <functional>

LogicNor::LogicNor(const int inputSize)
    : LogicElement(inputSize, 1, inputSize + 1)
{
}

void LogicNor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (isTempSimulationOn()) {
        const auto result = std::accumulate(m_inputBuffer.last().cbegin(), m_inputBuffer.last().cend(), false, std::bit_or<>());
        setOutputValue(!result);
        updateInputBuffer();
    } else {
        const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_or<>());
        setOutputValue(!result);
    }
}
