// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicxnor.h"

#include <functional>

LogicXnor::LogicXnor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXnor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_xor<>());
    setOutputValue(!result);
}
