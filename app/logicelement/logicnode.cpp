// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnode.h"

LogicNode::LogicNode()
    : LogicElement(1, 1)
{
}

void LogicNode::_updateLogic(const std::vector<bool> &inputs)
{
    setOutputValue(inputs[0]);
}