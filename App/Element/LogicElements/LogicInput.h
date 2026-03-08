// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for input driver nodes (VCC, GND, switch, etc.).
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicInput
 * \brief Logic element that drives one or more constant or controlled output values.
 *
 * \details Used for static drivers (VCC = true, GND = false) as well as
 * user-controlled switches and buttons.  The associated GraphicElementInput
 * calls setOutputValue() directly to push state changes into the simulation.
 */
class LogicInput : public LogicElement
{
public:
    /**
     * \brief Constructs a LogicInput with a fixed \a defaultValue on all outputs.
     * \param defaultValue Initial output value (true = logic-1, false = logic-0).
     * \param nOutputs     Number of output ports (default 1).
     */
    explicit LogicInput(const bool defaultValue = false, const int nOutputs = 1);

    /// No-op: input values are driven externally via setOutputValue().
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicInput)
};
