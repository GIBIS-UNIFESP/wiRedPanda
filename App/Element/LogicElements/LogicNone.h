// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief No-op logic element placeholder used by non-simulated elements.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicNone
 * \brief No-op logic element with zero inputs and zero outputs.
 *
 * \details Used as a placeholder for purely decorative elements (Line, Text)
 * that have no simulation behaviour.
 */
class LogicNone : public LogicElement
{
public:
    /// Constructs a LogicNone with 0 inputs and 0 outputs.
    explicit LogicNone() : LogicElement(0, 0) {}

private:
    Q_DISABLE_COPY(LogicNone)

    void updateLogic() override {}
};
