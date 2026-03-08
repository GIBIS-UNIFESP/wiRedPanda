// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the NAND gate.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicNand
 * \brief Logic element implementing the NAND gate: output = NOT(AND of all inputs).
 */
class LogicNand : public LogicElement
{
public:
    /**
     * \brief Constructs a LogicNand with \a inputSize inputs.
     * \param inputSize Number of input ports.
     */
    explicit LogicNand(const int inputSize);

    /// Sets output to the logical NAND of all inputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicNand)
};
