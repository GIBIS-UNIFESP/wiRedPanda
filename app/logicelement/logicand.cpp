// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicand.h"

#include <functional>

LogicAnd::LogicAnd(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicAnd::_updateLogic(const QVector<bool> &inputs)
{
    const auto result = std::accumulate(inputs.begin(), inputs.end(), true, std::bit_and<>());
    setOutputValue(result);
}
