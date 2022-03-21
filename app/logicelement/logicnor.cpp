// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnor.h"

#include <functional>

LogicNor::LogicNor(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicNor::_updateLogic(const std::vector<bool> &inputs)
{
    auto result = std::accumulate(inputs.begin(), inputs.end(), false, std::bit_or<bool>());
    setOutputValue(!result);
}