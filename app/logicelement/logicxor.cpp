// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicxor.h"

#include <functional>

LogicXor::LogicXor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_xor<>());
    setOutputValue(result);
}
