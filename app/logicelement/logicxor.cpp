// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicxor.h"

#include <functional>

LogicXor::LogicXor(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXor::_updateLogic(const std::vector<bool> &inputs)
{
    auto result = std::accumulate(inputs.begin(), inputs.end(), false, std::bit_xor<>());
    setOutputValue(result);
}
