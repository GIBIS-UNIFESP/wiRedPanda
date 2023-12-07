// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logictruthtable.h"

#include <functional>

LogicTruthTable::LogicTruthTable(const int inputSize, const QBitArray& key)
    : LogicElement(inputSize, 1)
{
    proposition = key;
}

void LogicTruthTable::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    const auto pos = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), QString(""), [](QString acc, bool b){acc+= b == 1 ? '1' : '0'; return acc;}).toUInt(nullptr, 2);
    const bool result = proposition.at(pos);
    setOutputValue(result);
}
