// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicDemux.h"

// Helper to calculate required select lines for output count
static int calculateSelectLines(int outputSize)
{
    int selectLines = 1;
    while ((1 << selectLines) < outputSize) {
        selectLines++;
    }
    return selectLines;
}

LogicDemux::LogicDemux(int outputSize)
    : LogicElement(1 + calculateSelectLines(outputSize), outputSize)
{
    m_numSelectLines = calculateSelectLines(outputSize);
    m_numOutputs = outputSize;
}

void LogicDemux::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    const Status data = inputs().at(0);

    int selectValue = 0;
    for (int i = 0; i < m_numSelectLines; i++) {
        const Status sel = inputs().at(1 + i);
        if (sel != Status::Active && sel != Status::Inactive) {
            const Status err = (sel == Status::Error) ? Status::Error : Status::Unknown;
            for (int j = 0; j < m_numOutputs; j++) {
                setOutputValue(j, err);
            }
            return;
        }
        if (sel == Status::Active) {
            selectValue |= (1 << i);
        }
    }

    for (int i = 0; i < m_numOutputs; i++) {
        setOutputValue(i, (i == selectValue) ? data : Status::Inactive);
    }
}
