// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicXor.h"

#include <functional>

LogicXor::LogicXor(const int inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicXor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    // XOR fold with identity=false; for n inputs this computes odd-parity
    // (true when an odd number of inputs are high), which generalises the
    // 2-input XOR definition consistently across all gate widths.
    const auto result = std::accumulate(inputs().cbegin(), inputs().cend(), false, std::bit_xor<>());
    setOutputValue(result);
}

