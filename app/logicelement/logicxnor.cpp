// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicxnor.h"

#include <cmath>
#include <functional>

LogicXnor::LogicXnor(const int inputSize)
    : LogicElement(inputSize, 1, std::pow(2, inputSize - 1) + 4)
{
}

void LogicXnor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (isTempSimulationOn()) {
        updateInputBuffer();
        const auto result = std::accumulate(m_inputBuffer.last().cbegin(), m_inputBuffer.last().cend(), false, std::bit_xor<>());
        setOutputValue(!result);
    } else {
        const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_xor<>());
        setOutputValue(!result);
    }
}
