// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicMux.h"

LogicMux::LogicMux(int inputSize)
    : LogicElement(inputSize, 1)
{
    // Calculate minimum select lines needed for the given data input count
    // We need: 2^m_numSelectLines >= m_numDataInputs, where m_numDataInputs = inputSize - m_numSelectLines
    m_numSelectLines = 1;
    while (true) {
        m_numDataInputs = inputSize - m_numSelectLines;
        if ((1 << m_numSelectLines) >= m_numDataInputs) {
            break;
        }
        m_numSelectLines++;
    }
}

void LogicMux::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    int selectValue = 0;
    for (int i = 0; i < m_numSelectLines; i++) {
        if (inputs().at(m_numDataInputs + i)) {
            selectValue |= (1 << i);
        }
    }

    setOutputValue(inputs().at(selectValue));
}

