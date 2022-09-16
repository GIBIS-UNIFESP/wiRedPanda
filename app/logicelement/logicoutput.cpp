// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicoutput.h"

LogicOutput::LogicOutput(const int inputSize)
    : LogicElement(inputSize, inputSize)
{
}

void LogicOutput::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    for (int index = 0; index < m_inputValues.size(); ++index) {
        setOutputValue(index, m_inputValues.at(index));
    }
}
