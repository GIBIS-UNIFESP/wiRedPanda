// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicxor.h"

LogicXor::LogicXor(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXor::_updateLogic(const std::vector<bool> &inputs)
{
    bool result = false;
    for (bool in : inputs) {
        result ^= in;
    }
    setOutputValue(result);
}