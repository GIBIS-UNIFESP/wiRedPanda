// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Four-state logic operations for gate evaluation.
 *
 * Domination rules (Unknown = not yet resolved, Error = bus conflict):
 *   - AND: Inactive dominates (FALSE AND X = FALSE)
 *   - OR:  Active dominates (TRUE OR X = TRUE)
 *   - NOT: inverts known values; Unknown -> Unknown, Error -> Error
 *   - XOR: requires all inputs known; any unknown -> Unknown/Error
 *
 * Unknown propagates benignly as "not yet resolved" and resolves once
 * a dominant known value arrives via event-driven propagation.
 */

#pragma once

#include <QVector>

#include "App/Core/Enums.h"

namespace StatusOps {

/// AND: Inactive dominates all; unknown propagates as-is.
inline Status statusAnd(const Status a, const Status b)
{
    if (a == Status::Inactive || b == Status::Inactive) {
        return Status::Inactive;
    }
    if (a == Status::Active && b == Status::Active) {
        return Status::Active;
    }
    if (a == Status::Error || b == Status::Error) {
        return Status::Error;
    }
    return Status::Unknown;
}

/// OR: Active dominates all; unknown propagates as-is.
inline Status statusOr(const Status a, const Status b)
{
    if (a == Status::Active || b == Status::Active) {
        return Status::Active;
    }
    if (a == Status::Inactive && b == Status::Inactive) {
        return Status::Inactive;
    }
    if (a == Status::Error || b == Status::Error) {
        return Status::Error;
    }
    return Status::Unknown;
}

/// NOT: inverts known values; Unknown -> Unknown, Error -> Error.
inline Status statusNot(const Status a)
{
    if (a == Status::Active) {
        return Status::Inactive;
    }
    if (a == Status::Inactive) {
        return Status::Active;
    }
    return a;
}

/// XOR: requires all inputs known; any Unknown -> Unknown, any Error -> Error.
inline Status statusXor(const Status a, const Status b)
{
    if ((a == Status::Active || a == Status::Inactive)
        && (b == Status::Active || b == Status::Inactive)) {
        return (a != b) ? Status::Active : Status::Inactive;
    }
    if (a == Status::Error || b == Status::Error) {
        return Status::Error;
    }
    return Status::Unknown;
}

/// Fold AND over a range. Inactive dominates.
inline Status statusAndAll(const QVector<Status> &inputs)
{
    bool hasUnknown = false;
    bool hasError = false;
    for (const auto s : inputs) {
        if (s == Status::Inactive) {
            return Status::Inactive;
        }
        if (s == Status::Error) {
            hasError = true;
        } else if (s == Status::Unknown) {
            hasUnknown = true;
        }
    }
    if (hasError) { return Status::Error; }
    return hasUnknown ? Status::Unknown : Status::Active;
}

/// Fold OR over a range. Active dominates.
inline Status statusOrAll(const QVector<Status> &inputs)
{
    bool hasUnknown = false;
    bool hasError = false;
    for (const auto s : inputs) {
        if (s == Status::Active) {
            return Status::Active;
        }
        if (s == Status::Error) {
            hasError = true;
        } else if (s == Status::Unknown) {
            hasUnknown = true;
        }
    }
    if (hasError) { return Status::Error; }
    return hasUnknown ? Status::Unknown : Status::Inactive;
}

/// Fold XOR (odd-parity) over a range. Any Unknown -> Unknown, any Error -> Error.
/// Error takes priority over Unknown (consistent with statusAndAll/statusOrAll).
inline Status statusXorAll(const QVector<Status> &inputs)
{
    int activeCount = 0;
    bool hasUnknown = false;
    for (const auto s : inputs) {
        if (s == Status::Active) {
            ++activeCount;
        } else if (s == Status::Error) {
            return Status::Error;
        } else if (s == Status::Unknown) {
            hasUnknown = true;
        }
    }
    if (hasUnknown) { return Status::Unknown; }
    return (activeCount % 2 != 0) ? Status::Active : Status::Inactive;
}

} // namespace StatusOps

