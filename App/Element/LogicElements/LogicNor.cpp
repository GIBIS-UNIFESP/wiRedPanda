// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicNor.h"

#include "App/Core/StatusOps.h"

LogicNor::LogicNor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicNor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    setOutputValue(StatusOps::statusNot(StatusOps::statusOrAll(inputs())));
}
