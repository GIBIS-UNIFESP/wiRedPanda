// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicNode.h"

LogicNode::LogicNode()
    : LogicElement(1, 1)
{
}

void LogicNode::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // A node is a wire junction: it exists solely to fan out a single signal to
    // multiple destinations without any logical transformation.
    setOutputValue(m_inputValues.at(0));
}

