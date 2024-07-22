// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnand.h"

#include <functional>

LogicNand::LogicNand(const int inputSize)
    : LogicElement(inputSize, 1, inputSize + 1)
{
}

void LogicNand::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (isTempSimulationOn()) {
        const auto result = std::accumulate(m_inputBuffer.last().cbegin(), m_inputBuffer.last().cend(), true, std::bit_and<>());
        setOutputValue(!result);
        updateInputBuffer();
    } else {
        const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), true, std::bit_and<>());
        setOutputValue(!result);
    }
}
