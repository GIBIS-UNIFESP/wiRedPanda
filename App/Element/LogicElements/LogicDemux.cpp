// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicDemux.h"

LogicDemux::LogicDemux()
    : LogicElement(2, 2)
{
}

void LogicDemux::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    const bool data = m_inputValues.at(0);
    const bool choice = m_inputValues.at(1);

    // Non-selected outputs must be explicitly forced to 0; they don't retain
    // their previous value (unlike a latch).
    bool out0 = false;
    bool out1 = false;

    // choice=0 routes to output 0; choice=1 routes to output 1.
    if (!choice) {
        out0 = data;
    } else {
        out1 = data;
    }

    setOutputValue(0, out0);
    setOutputValue(1, out1);
}

