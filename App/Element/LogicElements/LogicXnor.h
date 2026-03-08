// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the XNOR gate.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicXnor
 * \brief Logic element implementing the XNOR gate: output = NOT XOR of all inputs.
 */
class LogicXnor : public LogicElement
{
public:
    /**
     * \brief Constructs a LogicXnor with \a inputSize inputs.
     * \param inputSize Number of input ports.
     */
    explicit LogicXnor(const int inputSize);

    /// Sets output to the logical XNOR (even-parity) of all inputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicXnor)
};
