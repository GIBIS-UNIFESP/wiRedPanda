// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicNone : public LogicElement
{
public:
    explicit LogicNone() : LogicElement(0, 0) {}

private:
    Q_DISABLE_COPY(LogicNone)

    void updateLogic() override {}
};
