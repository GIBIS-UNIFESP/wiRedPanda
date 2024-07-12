// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnone.h"

LogicNone::LogicNone()
    : LogicElement(0, 0)
{
}

void LogicNone::_updateLogic(const std::vector<bool> &inputs)
{
    Q_UNUSED(inputs);
}

void LogicNone::notifyNextElement(){
    for (auto* next : getSuccessors()) {
        next->setReady(true);
    }
}
