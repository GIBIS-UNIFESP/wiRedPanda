// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicTruthTable.h"

LogicTruthTable::LogicTruthTable(const int inputSize, const int outputSize, const QBitArray &key)
    : LogicElement(inputSize, outputSize)
{
    m_proposition = key;
    m_nOutputs = outputSize;
}

void LogicTruthTable::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    for (int i = 0; i < this->m_nOutputs; ++i) {
        // Convert the input bit vector to a row index by concatenating "0"/"1"
        // characters and parsing as a binary number; this gives the row of the
        // truth table that corresponds to the current input combination.
        // String-based binary conversion is used for readability; with at most
        // 8 inputs the string is at most 8 characters long, so performance is fine.
        const auto pos = std::accumulate(inputs().cbegin(), inputs().cend(), QString(""),
                                         [](QString acc, Status s) {
                                             acc += (s == Status::Active) ? '1' : '0';
                                             return acc;
                                         }).toUInt(nullptr, 2);
        // The proposition QBitArray stores all output columns in a flat layout.
        // 256 = 2^8, the maximum number of rows for the maximum 8 input pins.
        // Each output column i occupies a 256-entry block regardless of actual
        // input count, so unused rows at the end of each block are simply ignored.
        const bool result = m_proposition.at(static_cast<std::size_t>(256 * i) + static_cast<std::size_t>(pos));
        setOutputValue(i, result ? Status::Active : Status::Inactive);
    }
}
