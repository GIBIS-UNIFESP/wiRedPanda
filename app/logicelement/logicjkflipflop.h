// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicJKFlipFlop : public LogicElement
{
public:
    explicit LogicJKFlipFlop();

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicJKFlipFlop)

    bool m_lastClk = false;
    bool m_lastJ = true;
    bool m_lastK = true;
};
