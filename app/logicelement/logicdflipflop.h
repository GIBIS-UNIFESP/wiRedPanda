// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicDFlipFlop : public LogicElement
{
public:
    explicit LogicDFlipFlop();

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicDFlipFlop)

    bool m_lastClk = false;
    bool m_lastValue = true;
};
