// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicor.h"

#include <functional>

LogicOr::LogicOr(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicOr::_updateLogic(const QVector<bool> &inputs)
{
    const auto result = std::accumulate(inputs.begin(), inputs.end(), false, std::bit_or<>());
    setOutputValue(result);
}
