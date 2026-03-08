// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the XOR gate.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicXor
 * \brief Logic element implementing the XOR gate: output = parity of all inputs.
 */
class LogicXor : public LogicElement
{
public:
    /**
     * \brief Constructs a LogicXor with \a inputSize inputs.
     * \param inputSize Number of input ports.
     */
    explicit LogicXor(const int inputSize);

    /// Sets output to the logical XOR (odd-parity) of all inputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicXor)
};
