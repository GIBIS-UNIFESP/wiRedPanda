// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the D latch.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicDLatch
 * \brief Level-sensitive D latch logic element.
 *
 * \details Inputs (in order): D, Enable.  Outputs: Q, Q̄.
 * When Enable is high, Q follows D continuously.
 */
class LogicDLatch : public LogicElement
{
public:
    /// Constructs a LogicDLatch with 2 inputs and 2 outputs.
    explicit LogicDLatch();

    /// Updates Q and Q̄ based on D and the enable level.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicDLatch)
};
