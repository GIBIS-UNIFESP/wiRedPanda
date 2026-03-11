// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicOutput.h"

LogicOutput::LogicOutput(const int inputSize)
    : LogicElement(inputSize, inputSize) // output count mirrors input count so the
                                         // UI layer can read each channel independently
{
}

void LogicOutput::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // Mirror every input directly to the matching output port so that the
    // display layer can query outputValue(i) without accessing the graph edges.
    for (int index = 0; index < m_inputValues.size(); ++index) {
        setOutputValue(index, m_inputValues.at(index));
    }
}
