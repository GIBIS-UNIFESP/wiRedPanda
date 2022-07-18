// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicdemux.h"

LogicDemux::LogicDemux()
    : LogicElement(2, 2)
{
}

void LogicDemux::_updateLogic(const QVector<bool> &inputs)
{
    const bool data = inputs.at(0);
    const bool choice = inputs.at(1);

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
