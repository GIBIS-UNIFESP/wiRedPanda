// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicMux.h"

LogicMux::LogicMux()
    : LogicElement(3, 1)
{
}

void LogicMux::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // Input layout: data0 at index 0, data1 at index 1, select at index 2.
    // This matches the pin ordering defined in the graphic element.
    const bool data1 = m_inputValues.at(0);
    const bool data2 = m_inputValues.at(1);
    const bool choice = m_inputValues.at(2);

    // choice=0 → forward data1 (input 0); choice=1 → forward data2 (input 1).
    setOutputValue(choice ? data2 : data1);
}
