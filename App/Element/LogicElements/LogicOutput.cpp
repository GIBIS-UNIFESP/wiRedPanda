// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicOutput.h"

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

