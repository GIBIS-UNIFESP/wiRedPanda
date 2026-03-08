// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the AND gate.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicAnd
 * \brief Logic element implementing the AND gate: output = AND of all inputs.
 */
class LogicAnd : public LogicElement
{
public:
    /**
     * \brief Constructs a LogicAnd with \a inputSize inputs.
     * \param inputSize Number of input ports (2–8).
     */
    explicit LogicAnd(const int inputSize);

    /// Sets output to the logical AND of all inputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicAnd)
};
