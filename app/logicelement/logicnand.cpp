// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnand.h"

#include <functional>

LogicNand::LogicNand(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicNand::_updateLogic(const QVector<bool> &inputs)
{
    const auto result = std::accumulate(inputs.begin(), inputs.end(), true, std::bit_and<>());
    setOutputValue(!result);
}
