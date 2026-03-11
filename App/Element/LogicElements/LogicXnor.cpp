// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicXnor.h"

#include <functional>

LogicXnor::LogicXnor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXnor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // XOR fold with identity=false counts parity; invert for XNOR (even-parity → true).
    // For more than 2 inputs this is an odd-parity detector negated, which is the
    // standard n-input XNOR convention used in this simulator.
    const auto result = std::accumulate(m_inputValues.cbegin(), m_inputValues.cend(), false, std::bit_xor<>());
    setOutputValue(!result);
}

