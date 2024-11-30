// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logictruthtable.h"

LogicTruthTable::LogicTruthTable(const int inputSize, const int outputSize, const QBitArray& key)
    : LogicElement(inputSize, outputSize, inputSize)
{
    m_proposition = key;
    m_nOutputs = outputSize;
}

void LogicTruthTable::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (isTempSimulationOn()) {
        for (int i = 0; i < this->m_nOutputs; i++)
        {
            const auto pos = std::accumulate(m_inputBuffer.last().cbegin(), m_inputBuffer.last().cend(), QString(""),
                                             [](QString acc, bool b) {
                                                 acc += (b == 1) ? '1' : '0';
                                                 return acc;
                                             }).toUInt(nullptr, 2);
            const bool result = m_proposition.at(256 * i + pos);
            setOutputValue(i, result);
        }
        updateInputBuffer();
    } else {
        for (int i = 0; i < this->m_nOutputs; i++)
        {
        const auto pos = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), QString(""),
                                         [](QString acc, bool b) {
                                             acc += (b == 1) ? '1' : '0';
                                             return acc;
                                         }).toUInt(nullptr, 2);
            const bool result = m_proposition.at(256 * i + pos);
        setOutputValue(i, result);
        }
    }
}
