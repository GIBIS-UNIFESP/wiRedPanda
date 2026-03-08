// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for demultiplexers.
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicDemux
 * \brief Logic element implementing a 1-to-2^N demultiplexer.
 *
 * \details Input layout: [data input (0), select lines (1..log2(N))].
 * The data input is routed to the output selected by the select lines;
 * all other outputs are set to logic-0.
 */
class LogicDemux : public LogicElement
{
public:
    /// Constructs the demultiplexer with its default input/output ports.
    explicit LogicDemux();

    /// Routes the data input to the output selected by the select lines.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicDemux)

    int m_numOutputs = 2;      ///< Number of data outputs.
    int m_numSelectLines = 1;  ///< Number of select-line inputs.
};
