// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicmux.h"

LogicMux::LogicMux()
    : LogicElement(3, 1)
{
}

void LogicMux::_updateLogic(const QVector<bool> &inputs)
{
    const bool data1 = inputs.at(0);
    const bool data2 = inputs.at(1);
    const bool choice = inputs.at(2);

    setOutputValue(choice ? data2 : data1);
}
