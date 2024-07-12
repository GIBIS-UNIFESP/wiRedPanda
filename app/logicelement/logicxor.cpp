// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicxor.h"

#include <cmath>
#include <functional>

LogicXor::LogicXor(const int inputSize)
    : LogicElement(inputSize, 1, std::pow(2, inputSize - 1) + 3)
{
}

void LogicXor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (!isTempSimulationOn()) {
        const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_xor<>());
        setOutputValue(result);
    }
    else {
        updateInputBuffer();
        const auto result = std::accumulate(inputBuffer.last().cbegin(), inputBuffer.last().cend(), false, std::bit_xor<>());
        setOutputValue(result);
    }
}

