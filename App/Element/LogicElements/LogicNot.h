// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for the NOT (inverter) gate.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicNot
 * \brief Logic element implementing a single-input inverter: output = NOT input.
 */
class LogicNot : public LogicElement
{
public:
    /// Constructs a LogicNot with 1 input and 1 output.
    explicit LogicNot();

    /// Sets output to the logical NOT of the single input.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicNot)
};
