// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the SR latch.
 */

#pragma once

#include "App/Element/LogicElement.h"

/**
 * \class LogicSRLatch
 * \brief Level-sensitive SR latch logic element.
 *
 * \details Inputs (active-low): S̄, R̄.  Outputs: Q, Q̄.
 */
class LogicSRLatch : public LogicElement
{
public:
    /// Constructs a LogicSRLatch with 2 inputs and 2 outputs.
    explicit LogicSRLatch();

    /// Updates Q and Q̄ based on active-low S and R inputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicSRLatch)
};

