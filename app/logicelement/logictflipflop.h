// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicTFlipFlop : public LogicElement
{
public:
    explicit LogicTFlipFlop();

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicTFlipFlop)

    bool m_lastClk = false;
    bool m_lastValue = true;
};
