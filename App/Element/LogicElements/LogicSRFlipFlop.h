// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the clocked SR flip-flop.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicSRFlipFlop
 * \brief Edge-triggered SR flip-flop logic element.
 *
 * \details Inputs: S, R, Clock, Preset (active-low), Clear (active-low).
 * Outputs: Q, Q̄.  State changes on the rising edge of Clock.
 */
class LogicSRFlipFlop : public LogicElement
{
public:
    // --- Lifecycle ---

    /// Constructs the flip-flop with its fixed input/output ports.
    explicit LogicSRFlipFlop();

    // --- Logic ---

    /// \reimp Updates output state on each rising clock edge.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicSRFlipFlop)

    // --- Members ---

    Status m_lastClk = Status::Inactive;
};
