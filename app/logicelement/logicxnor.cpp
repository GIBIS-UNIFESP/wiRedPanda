// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
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
