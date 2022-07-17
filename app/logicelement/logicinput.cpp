// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicinput.h"

LogicInput::LogicInput(const bool defaultValue, const int nOutputs)
    : LogicElement(0, nOutputs)
{
    setOutputValue(0, defaultValue);

    for (int port = 1; port < nOutputs; ++port) {
        setOutputValue(port, false);
    }
}

void LogicInput::_updateLogic(const QVector<bool> &inputs)
{
    Q_UNUSED(inputs);
}
