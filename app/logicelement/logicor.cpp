// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicor.h"

LogicOr::LogicOr(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicOr::_updateLogic(const std::vector<bool> &inputs)
{
    auto result = std::accumulate(inputs.begin(), inputs.end(), false, std::bit_or<bool>());
    setOutputValue(result);
}