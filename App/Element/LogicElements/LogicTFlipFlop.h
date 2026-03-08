// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the T flip-flop.
 */

#pragma once

#include "App/Element/LogicElement.h"

/**
 * \class LogicTFlipFlop
 * \brief Edge-triggered T (toggle) flip-flop logic element.
 *
 * \details Inputs: T, Clock, Preset (active-low), Clear (active-low).
 * Outputs: Q, Q̄.  Toggles Q on each rising clock edge when T is high.
 */
class LogicTFlipFlop : public LogicElement
{
public:
    // --- Lifecycle ---

    /// Constructs the flip-flop with its fixed input/output ports.
    explicit LogicTFlipFlop();

    // --- Logic ---

    /// \reimp Updates output state on each rising clock edge.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicTFlipFlop)

    // --- Members ---

    bool m_lastClk = false;
    bool m_lastValue = true;
};

