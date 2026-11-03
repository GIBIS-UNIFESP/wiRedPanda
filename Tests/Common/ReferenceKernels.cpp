// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/ReferenceKernels.h"

namespace {

/// The gate of \a type at levels: \a bits are its inputs, one level each.
bool levelOf(const ElementType type, const QVector<bool> &bits)
{
    bool all = true;
    bool any = false;
    bool parity = false;
    for (const bool bit : bits) {
        all = all && bit;
        any = any || bit;
        parity = parity != bit;
    }
    switch (type) {
    case ElementType::And:
        return all;
    case ElementType::Or:
        return any;
    case ElementType::Nand:
        return !all;
    case ElementType::Nor:
        return !any;
    case ElementType::Xor:
        return parity;
    case ElementType::Xnor:
        return !parity;
    case ElementType::Not:
        return !bits.first();
    default:
        return false; // LCOV_EXCL_LINE -- referenceGate() is called for the gates it names only
    }
}

} // namespace

bool atMostInFlatOrder(const Status a, const Status b)
{
    return (a == b) || (a == Status::Unknown);
}

Status referenceGate(const ElementType type, const QVector<Status> &inputs)
{
    // The inputs that are not levels, each read both ways: a reading is one choice of level for each of them
    QVector<int> open;
    for (int index = 0; index < inputs.size(); ++index) {
        if ((inputs.at(index) == Status::Unknown) || (inputs.at(index) == Status::Error)) {
            open.append(index);
        }
    }
    const auto bitsOf = [&inputs, &open](const int reading) {
        QVector<bool> bits(inputs.size());
        for (int index = 0; index < inputs.size(); ++index) {
            bits[index] = (inputs.at(index) == Status::Active);
        }
        for (int slot = 0; slot < open.size(); ++slot) {
            bits[open.at(slot)] = ((reading >> slot) & 1) != 0;
        }
        return bits;
    };

    bool sawInactive = false;
    bool sawActive = false;
    for (int reading = 0; reading < (1 << open.size()); ++reading) {
        (levelOf(type, bitsOf(reading)) ? sawActive : sawInactive) = true;
    }
    if (!(sawInactive && sawActive)) {
        return sawActive ? Status::Active : Status::Inactive;
    }

    // The readings disagree. An Unknown input decides the gate when some setting of the other open inputs makes its own
    // level change the output; then the gate is Unknown, and otherwise the disagreement is Error's alone.
    for (int unknown : open) {
        if (inputs.at(unknown) != Status::Unknown) {
            continue;
        }
        for (int reading = 0; reading < (1 << open.size()); ++reading) {
            QVector<bool> low = bitsOf(reading);
            QVector<bool> high = low;
            low[unknown] = false;
            high[unknown] = true;
            if (levelOf(type, low) != levelOf(type, high)) {
                return Status::Unknown;
            }
        }
    }
    return Status::Error;
}
