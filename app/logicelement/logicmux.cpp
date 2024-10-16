
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicmux.h"

LogicMux::LogicMux()
    : LogicElement(3, 1)
{
}

void LogicMux::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    const bool data1 = m_inputValues.at(0);
    const bool data2 = m_inputValues.at(1);
    const bool choice = m_inputValues.at(2);

    setOutputValue(choice ? data2 : data1);
}
