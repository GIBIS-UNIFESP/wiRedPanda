// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the D flip-flop.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicDFlipFlop
 * \brief Edge-triggered D flip-flop logic element.
 *
 * \details Inputs (in order): D, Clock, Preset (active-low), Clear (active-low).
 * Outputs: Q, Q̄.  State changes on the rising edge of Clock.
 */
class LogicDFlipFlop : public LogicElement
{
public:
    // --- Lifecycle ---

    /// Constructs the flip-flop with its fixed input/output ports.
    explicit LogicDFlipFlop();

    // --- Logic ---

    /// \reimp Updates output state on each rising clock edge.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicDFlipFlop)

    // --- Members ---

    Status m_lastClk = Status::Inactive;
    Status m_lastValue = Status::Active; ///< D sampled before the current event batch.
};
