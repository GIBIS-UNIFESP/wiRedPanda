// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ClockTiming.h"

#include "App/Element/Element.h"

namespace ClockTimingDetail {

/// The half-period of the frequency of \a element.
std::chrono::microseconds halfPeriodOf(const Element &element)
{
    return ClockTiming::halfPeriod(element.param<double>(ParamId::Frequency));
}

} // namespace ClockTimingDetail

namespace ClockTiming {

using ClockTimingDetail::halfPeriodOf;

void reset(Element &element, std::chrono::steady_clock::time_point globalTime)
{
    // Start clocks in the HIGH state; the first transition happens after one half-period
    element.setOn(true);
    // The delay is a fraction of the period (-1 to 1).
    // Negative delays advance the clock (trigger earlier), positive delays delay it.
    // Shifting the phase's start backward by the delay fraction effectively phase-shifts the waveform.
    // Full period is twice the half-period.
    const auto fullPeriod = 2 * halfPeriodOf(element);
    const double delay = element.param<double>(ParamId::Delay);
    const auto delayMicroseconds = static_cast<std::chrono::microseconds::rep>(-delay * static_cast<double>(fullPeriod.count()));
    ElementSimState::ClockPhase &phase = element.clockPhase();
    phase.start = globalTime;
    phase.start -= std::chrono::microseconds(delayMicroseconds);
    phase.set = true;
}

bool hasPhase(const Element &element)
{
    return element.clockPhase().set;
}

void shift(Element &element, std::chrono::steady_clock::duration pause)
{
    // Resume-after-pause: the wall clock advanced by `pause` while the simulation was
    // stopped, so move the phase reference forward by the same amount. update() then
    // sees the same elapsed-within-interval it saw at the moment of the pause — the output
    // level is untouched (no forced HIGH, no injected edge) and no missed toggles fire in
    // a burst.
    element.clockPhase().start += pause;
}

void update(Element &element, std::chrono::steady_clock::time_point globalTime)
{
    if (element.isLocked()) {
        return;
    }

    ElementSimState::ClockPhase &phase = element.clockPhase();
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(globalTime - phase.start);

    // A gap this large means the wall clock jumped far ahead without an intervening
    // Simulation::stop()/start() cycle (which already resyncs via reset() — see its call
    // site) — e.g. the OS suspended the process while the simulation's QTimer stayed "active".
    // Left to the single-step path below, replaying the backlog would toggle once per
    // subsequent 1 ms tick until caught up: for a long gap, a burst of thousands of rapid,
    // unrealistic toggles feeding into any connected sequential logic. Resync immediately
    // instead, on this very first tick after the gap, so the burst never starts. The threshold
    // is a fixed wall-clock duration, not a multiple of the half-period — scaling it to the
    // half-period would fire this path on entirely ordinary jitter for high-frequency clocks
    // (e.g. a 100 Hz clock's 5 ms interval means even a benign 20+ ms hiccup would look like a
    // "large" multiple), which is not the gap this guards against.
    constexpr auto kMaxCatchUpGap = std::chrono::seconds(5);
    if (elapsed > kMaxCatchUpGap) {
        reset(element, globalTime);
        return;
    }

    // The half-period is the time per HIGH or LOW phase.
    // Rather than resetting to globalTime we advance by exactly one half-period so
    // that accumulated drift doesn't skew the clock frequency over time.
    if (const auto interval = halfPeriodOf(element); elapsed > interval) {
        phase.start += interval;
        element.setOn(!element.isOn());
    }
}

} // namespace ClockTiming
