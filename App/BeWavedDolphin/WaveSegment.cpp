// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/WaveSegment.h"

#include "App/Core/Enums.h"

WaveSegment segmentFor(const int value, const bool hasPrev, const int prevValue)
{
    // Cells carry a four-state Status, not a bit: Unknown is -1 and Error is 2. Treating the
    // cell as "0 is low, everything else is high" would draw an undefined signal as a confident
    // logic HIGH, while the canvas renders that same signal as a distinct grey wire
    // (Connection.cpp / theme.m_connectionUnknown). Give them their own segments so the two views
    // agree -- routinely reachable, since the engine canonicalises oscillating regions to Unknown
    // and propagates it to every reader.
    if (value == static_cast<int>(Status::Unknown)) {
        return WaveSegment::Unknown;
    }
    if (value == static_cast<int>(Status::Error)) {
        return WaveSegment::Error;
    }
    if (value == 0) {
        return (hasPrev && (prevValue == 1)) ? WaveSegment::Falling : WaveSegment::Low;
    }

    return (hasPrev && (prevValue == 0)) ? WaveSegment::Rising : WaveSegment::High;
}
