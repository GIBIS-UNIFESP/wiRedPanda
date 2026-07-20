// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/SignalDelegate.h"

namespace SignalDelegate {

WaveSegment segmentFor(const int value, const bool hasPrev, const int prevValue)
{
    if (value == 0) {
        return (hasPrev && (prevValue == 1)) ? WaveSegment::Falling : WaveSegment::Low;
    }

    return (hasPrev && (prevValue == 0)) ? WaveSegment::Rising : WaveSegment::High;
}

} // namespace SignalDelegate
