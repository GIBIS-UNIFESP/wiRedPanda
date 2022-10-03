// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicdemux.h"

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

    bool out0 = false;
    bool out1 = false;

    if (!choice) {
        out0 = data;
    } else {
        out1 = data;
    }

    setOutputValue(0, out0);
    setOutputValue(1, out1);
}
