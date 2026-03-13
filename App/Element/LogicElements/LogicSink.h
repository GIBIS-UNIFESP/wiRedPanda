// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for output sink nodes (LEDs, displays, etc.).
 */

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

/**
 * \class LogicSink
 * \brief Logic element that sinks and reflects input values as outputs (for observation).
 *
 * \details Output elements such as LEDs read their state through this element.
 * updateLogic() copies each input value to the corresponding output so the
 * graphic element can query outputValue() to refresh its appearance.
 */
class LogicSink : public LogicElement
{
public:
    /**
     * \brief Constructs a LogicSink with \a inputSize inputs (and matching outputs).
     * \param inputSize Number of input ports.
     */
    explicit LogicSink(const int inputSize);

    /// Propagates input values to matching outputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicSink)
};
