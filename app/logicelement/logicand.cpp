// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicand.h"

LogicAnd::LogicAnd(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicAnd::_updateLogic(const std::vector<bool> &inputs)
{
    auto result = std::accumulate(inputs.begin(), inputs.end(), true, std::bit_and<bool>());
    setOutputValue(result);
}
