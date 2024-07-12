// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnode.h"

LogicNode::LogicNode()
    : LogicElement(1, 1)
{
}

void LogicNode::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    setOutputValue(m_inputValues.at(0));
}

