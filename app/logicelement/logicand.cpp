// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicand.h"

#include <functional>

LogicAnd::LogicAnd(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicAnd::_updateLogic(const std::vector<bool> &inputs)
{
    auto result = std::accumulate(inputs.begin(), inputs.end(), true, std::bit_and<>());
    setOutputValue(result);
}
