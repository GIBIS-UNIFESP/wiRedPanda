// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for wire junction nodes.
 */

#pragma once

#include "App/Element/LogicElement.h"

/**
 * \class LogicNode
 * \brief Logic element for a wire junction (T-split) node: passes input to output unchanged.
 */
class LogicNode : public LogicElement
{
public:
    /// Constructs a LogicNode with 1 input and 1 output.
    explicit LogicNode();

    /// Copies the single input value to the single output.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicNode)
};

