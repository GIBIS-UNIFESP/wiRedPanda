// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicoutput.h"

LogicOutput::LogicOutput(size_t inputSz)
    : LogicElement(inputSz, inputSz)
{
}

void LogicOutput::_updateLogic(const std::vector<bool> &inputs)
{
    for (size_t idx = 0; idx < inputs.size(); ++idx) {
        setOutputValue(idx, inputs[idx]);
    }
}