// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Boolean minimizer for generating compact logic expressions from truth tables.
 */

#pragma once

#include <QVector>
#include <QString>

class BooleanMinimizer
{
public:
    /// Minimizes a single-output boolean function encoded as a truth table.
    ///
    /// The caller may pass either:
    /// - the full truth table of length 2^inputCount for a single output bit, or
    /// - a compact set of active minterms represented as a list of row indices.
    ///
    /// Returns a list of one or more canonical product terms, joined with OR.
    static QVector<QString> minimize(const QVector<int> &inputs,
                                     const QVector<int> &outputs,
                                     int inputCount,
                                     int outputIndex);
};
