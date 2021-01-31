// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnot.h"

LogicNot::LogicNot()
    : LogicElement(1, 1)
{
}

void LogicNot::_updateLogic(const std::vector<bool> &inputs)
{
    setOutputValue(!inputs[0]);
}