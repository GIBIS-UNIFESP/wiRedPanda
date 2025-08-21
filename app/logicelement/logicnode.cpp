// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnode.h"

LogicNode::LogicNode()
    : LogicElement(1, 1)
{
}

void LogicNode::updateLogic()
{
    if (!updateInputs()) {
        // For wireless nodes, try to use the output value as the source
        // This allows wireless receiving nodes to be valid even without physical inputs
        if (isValid()) {
            // Node is acting as a wireless signal source - keep current output value
            return;
        } else {
            // No valid inputs and no valid output - node is invalid
            return;
        }
    }

    setOutputValue(m_inputValues.at(0));
}
