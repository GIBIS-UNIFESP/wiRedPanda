// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnor.h"

LogicNor::LogicNor(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicNor::_updateLogic(const std::vector<bool> &inputs)
{
    bool result = false;
    for (bool in : inputs) {
        result |= in;
    }
    setOutputValue(!result);
}