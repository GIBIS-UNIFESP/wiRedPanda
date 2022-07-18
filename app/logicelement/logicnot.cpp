// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnot.h"

LogicNot::LogicNot()
    : LogicElement(1, 1)
{
}

void LogicNot::_updateLogic(const QVector<bool> &inputs)
{
    setOutputValue(!inputs.at(0));
}
