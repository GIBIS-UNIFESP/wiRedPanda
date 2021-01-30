// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicinput.h"

LogicInput::LogicInput(bool defaultValue)
    : LogicElement(0, 1)
{
    setOutputValue(0, defaultValue);
}

void LogicInput::_updateLogic(const std::vector<bool> &)
{
    // Does nothing on update
}