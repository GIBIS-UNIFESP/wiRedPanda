// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LogicNot.h"

LogicNot::LogicNot()
    : LogicElement(1, 1)
{
}

void LogicNot::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    setOutputValue(!m_inputValues.at(0));
}
