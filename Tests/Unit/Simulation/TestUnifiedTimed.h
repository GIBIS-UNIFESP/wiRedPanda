// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// Validates that the unified event-driven engine performs TIMED (propagation-delay)
/// simulation correctly — the same engine that reproduces zero-delay behavior.
class TestUnifiedTimed : public QObject
{
    Q_OBJECT

private slots:
    /// A NOT with delay N flips its output exactly N time-units after the input changes.
    void testGateDelay();

    /// Two chained NOTs accumulate their delays (second flips after delay1 + delay2).
    void testChainedDelay();

    /// An input pulse narrower than the gate delay is absorbed (inertial delay model).
    void testGlitchAbsorbed();

    /// A flip-flop clocked directly by another flip-flop's Q (a ripple/derived-clock chain,
    /// no gate in between) must react to the edge its predecessor produces, in both functional
    /// and temporal mode, on every edge — not just the first.
    void testRippleFlipFlopReevaluation();

    /// A structural edit (element freed + Simulation::initialize()) while temporal events are
    /// still pending must drop those events: they hold raw pointers into the old netlist, so
    /// draining one afterwards dereferences freed memory (use-after-free under ASan pre-fix).
    void testStructuralEditDropsPendingEvents();

    /// A time-base reset (Simulation::restart()) must clear the waveform recorder's recorded
    /// transition history (not just the sim clock), so a later transition never gets appended
    /// with a smaller timestamp than one already recorded.
    void testRestartResetsRecorderTimeline();

    /// A flip-flop's recorded Q transition must carry the tick boundary at which its deferred
    /// commit published (edge + delay at fine tick resolution) — not the timestamp of the next
    /// event-bearing tick, which for a slow clock is half a period late.
    void testSequentialTransitionRecordedAtCommitTime();
};
