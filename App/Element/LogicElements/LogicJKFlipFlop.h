// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the JK flip-flop.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicJKFlipFlop
 * \brief Edge-triggered JK flip-flop logic element.
 *
 * \details Inputs: J, K, Clock, Preset (active-low), Clear (active-low).
 * Outputs: Q, Q̄.  Toggle when both J and K are high on the rising clock edge.
 */
class LogicJKFlipFlop : public LogicElement
{
public:
    // --- Lifecycle ---

    /// Constructs the flip-flop with its fixed input/output ports.
    explicit LogicJKFlipFlop();

    // --- Logic ---

    /// \reimp Updates output state on each rising clock edge.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicJKFlipFlop)

    // --- Members ---

    Status m_lastClk = Status::Inactive;
    Status m_lastJ = Status::Active;
    Status m_lastK = Status::Active;
};
