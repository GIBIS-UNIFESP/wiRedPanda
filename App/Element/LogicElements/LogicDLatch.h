// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "LogicElement.h"

class LogicDLatch : public LogicElement
{
public:
    explicit LogicDLatch();

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicDLatch)
};
