// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for multiplexers.
 */

#pragma once

#include "App/Element/LogicElement.h"

/**
 * \class LogicMux
 * \brief Logic element implementing a 2^N-to-1 multiplexer.
 *
 * \details Input layout: [data inputs (0..N-1), select lines (N..N+log2(N)-1)].
 * The select lines choose which data input is forwarded to the output.
 */
class LogicMux : public LogicElement
{
public:
    /// Constructs the multiplexer with its default input/output ports.
    explicit LogicMux();

    /// Routes the data input selected by the select lines to the output.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicMux)

    int m_numDataInputs = 2;   ///< Number of data inputs.
    int m_numSelectLines = 1;  ///< Number of select-line inputs.
};

