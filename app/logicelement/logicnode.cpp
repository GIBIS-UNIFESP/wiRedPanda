// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnode.h"

#include <QDebug>

LogicNode::LogicNode()
    : LogicElement(1, 1)
{
}

// Virtual input methods removed - Option D uses actual QNEConnection objects
// for wireless connections, so no special virtual input handling needed

void LogicNode::updateLogic()
{
    // Option D: Standard QNEConnection handling - no special wireless logic needed
    // Wireless connections are actual QNEConnection objects, so they work identically to physical

    if (updateInputs()) {
        // Have input (physical or wireless connection) - use it
        setOutputValue(m_inputValues.at(0));
        return;
    }

    // No inputs - node keeps current state if valid (for source nodes)
    if (isValid()) {
        // Keep current output value
        return;
    }

    // Node has no inputs and is invalid - this shouldn't happen in normal operation
}
