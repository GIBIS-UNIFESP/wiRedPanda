// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the NOR gate.
 */

#pragma once

#include "App/Element/LogicElement.h"

/**
 * \class LogicNor
 * \brief Logic element implementing the NOR gate: output = NOT(OR of all inputs).
 */
class LogicNor : public LogicElement
{
public:
    /**
     * \brief Constructs a LogicNor with \a inputSize inputs.
     * \param inputSize Number of input ports.
     */
    explicit LogicNor(const int inputSize);

    /// Sets output to the logical NOR of all inputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicNor)
};

