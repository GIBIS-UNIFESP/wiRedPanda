// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicand.h"

#include <functional>

LogicAnd::LogicAnd(const int inputSize)
    : LogicElement(inputSize, 1, inputSize)
{
}

void LogicAnd::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (!isTempSimulationOn())    {
        const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), true, std::bit_and<>());
        setOutputValue(result);
    }
    else {
        updateInputBuffer();
        const auto result = std::accumulate(inputBuffer.last().cbegin(), inputBuffer.last().cend(), true, std::bit_and<>());
        setOutputValue(result);
    }
}
