// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnone.h"

LogicNone::LogicNone()
    : LogicElement(0,0)
{
}


void LogicNone::_updateLogic(const std::vector<bool> &inputs)
{
    Q_UNUSED(inputs);
}
