// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicxnor.h"

#include <functional>

LogicXnor::LogicXnor(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXnor::_updateLogic(const std::vector<bool> &inputs)
{
    auto result = std::accumulate(inputs.begin(), inputs.end(), false, std::bit_xor<bool>());
    setOutputValue(!result);
}