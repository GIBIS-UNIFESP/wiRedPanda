// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicoutput.h"

LogicOutput::LogicOutput(const int inputSize)
    : LogicElement(inputSize, inputSize)
{
}

void LogicOutput::_updateLogic(const QVector<bool> &inputs)
{
    for (int index = 0; index < inputs.size(); ++index) {
        setOutputValue(index, inputs[index]);
    }
}
