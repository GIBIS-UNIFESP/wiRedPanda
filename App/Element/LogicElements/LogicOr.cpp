// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicOr.h"

#include "App/Core/StatusOps.h"

LogicOr::LogicOr(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicOr::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    setOutputValue(StatusOps::statusOrAll(inputs()));
}
