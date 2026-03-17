// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicAnd.h"

#include "App/Core/StatusOps.h"

LogicAnd::LogicAnd(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicAnd::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    setOutputValue(StatusOps::statusAndAll(inputs()));
}
