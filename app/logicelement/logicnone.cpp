// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicnone.h"

LogicNone::LogicNone()
    : LogicElement(0, 0)
{
}

void LogicNone::updateLogic()
{
}

void LogicNone::notifyNextElement(){
    // for (auto *next : getSuccessors()) {
    //     next->setReady(true);
    // }
}
