// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnot.h"

LogicNot::LogicNot()
    : LogicElement(1, 1, 1)
{
}

void LogicNot::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (!isTempSimulationOn())
        setOutputValue(!m_inputValues.at(0));
    else {
        setOutputValue(!inputBuffer.last()[0]);
        updateInputBuffer();
    }
}
