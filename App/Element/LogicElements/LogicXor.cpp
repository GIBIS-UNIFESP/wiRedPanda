// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicXor.h"

#include "App/Core/StatusOps.h"

LogicXor::LogicXor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    setOutputValue(StatusOps::statusXorAll(inputs()));
}
