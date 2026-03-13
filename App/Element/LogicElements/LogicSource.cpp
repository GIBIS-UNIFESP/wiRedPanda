// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicSource.h"

LogicSource::LogicSource(const bool defaultValue, const int nOutputs)
    : LogicElement(0, nOutputs) // 0 inputs: source node, drives the graph boundary
{
    // Primary output carries the user-controlled value; secondary outputs (used
    // by multi-bit input devices such as keyboards) default to Inactive.
    setOutputValue(0, defaultValue ? Status::Active : Status::Inactive);

    for (int port = 1; port < nOutputs; ++port) {
        setOutputValue(port, Status::Inactive);
    }
}

void LogicSource::updateLogic()
{
    // Intentionally empty: the output value is set externally by the UI layer
    // (e.g. when the user toggles a switch), not computed from any inputs.
}
