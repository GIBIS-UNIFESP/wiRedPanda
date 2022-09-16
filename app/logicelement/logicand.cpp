// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicand.h"

#include <functional>

LogicAnd::LogicAnd(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicAnd::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), true, std::bit_and<>());
    setOutputValue(result);
}
