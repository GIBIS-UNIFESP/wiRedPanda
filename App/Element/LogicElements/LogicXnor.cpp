// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicXnor.h"

#include "App/Core/StatusOps.h"

LogicXnor::LogicXnor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXnor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    setOutputValue(StatusOps::statusNot(StatusOps::statusXorAll(inputs())));
}
