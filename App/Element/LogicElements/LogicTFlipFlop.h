// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/LogicElement.h"

class LogicTFlipFlop : public LogicElement
{
public:
    // --- Lifecycle ---

    explicit LogicTFlipFlop();

    // --- Logic ---

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicTFlipFlop)

    // --- Members ---

    bool m_lastClk = false;
    bool m_lastValue = true;
};

