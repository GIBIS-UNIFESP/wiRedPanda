// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
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

    if (isTempSimulationOn()) {
        updateInputBuffer();
        const auto result = std::accumulate(m_inputBuffer.last().cbegin(), m_inputBuffer.last().cend(), true, std::bit_and<>());
        setOutputValue(result);
    } else {
        const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), true, std::bit_and<>());
        setOutputValue(result);
    }
}
