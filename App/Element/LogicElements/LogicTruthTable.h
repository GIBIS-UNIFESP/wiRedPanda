// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Logic simulation element for user-programmable truth tables.
 */

#pragma once

#include <QBitArray>

#include "App/Element/LogicElement.h"

/**
 * \class LogicTruthTable
 * \brief Logic element whose behaviour is defined by a user-programmable truth table.
 *
 * \details The truth table is encoded as a flat QBitArray (the \e proposition) of
 * size 2^inputSize × outputSize.  updateLogic() indexes into the proposition using
 * the current input combination and sets each output accordingly.
 */
class LogicTruthTable : public LogicElement
{
public:
    // --- Lifecycle ---

    /**
     * \brief Constructs the truth table gate.
     * \param inputSize  Number of input bits.
     * \param outputSize Number of output bits.
     * \param key        Bit-array encoding all output combinations.
     */
    explicit LogicTruthTable(const int inputSize, const int outputSize, const QBitArray &key);

    // --- Logic ---

    /// \reimp Looks up the current input pattern in the truth table and drives outputs.
    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicTruthTable)

    // --- Members ---

    QBitArray m_proposition;
    int m_nOutputs;
};

