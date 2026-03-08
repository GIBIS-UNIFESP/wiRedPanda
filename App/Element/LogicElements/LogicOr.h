// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the OR gate.
 */

#pragma once

#include "App/Element/LogicElement.h"

/**
 * \class LogicOr
 * \brief Logic element implementing the OR gate: output = OR of all inputs.
 */
class LogicOr : public LogicElement
{
public:
    /**
     * \brief Constructs a LogicOr with \a inputSize inputs.
     * \param inputSize Number of input ports (2–8).
     */
    explicit LogicOr(const int inputSize);

    /// Sets output to the logical OR of all inputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicOr)
};

