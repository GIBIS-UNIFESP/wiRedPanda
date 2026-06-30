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

    /// A clocked D-flip-flop captures Data only on the rising clock edge, and its Q settles
    /// exactly one propagation delay after that edge (exercises the delayed-capture trick
    /// under event scheduling — the first temporal-mode test of sequential logic).
    void testFlipFlopClockedUnderDelay();

    /// A T-flip-flop (T=1) toggles Q exactly once per delayed rising clock edge — never twice
    /// per edge — confirming edge detection holds under per-element propagation delay.
    void testCounterBitTogglesUnderDelay();

    /// A 3-inverter ring with per-gate delays oscillates over time: a single wide tick drains a
    /// run of transitions at distinct, evenly spaced timestamps (it never hits the per-timestamp
    /// oscillation cap, unlike the functional zero-delay ring which canonicalizes to Unknown).
    void testRingOscillatorTemporal();

    /// A gated clock (enable AND master-clock) feeding a downstream flip-flop produces no
    /// spurious clocking when the enable and clock edges collide at one timestamp — the
    /// (time, priority-desc) drain settles the gate before the flip-flop samples it.
    void testGatedClockNoGlitchTemporal();

    /// Two independent same-delay branches from a shared source collide at the exact same
    /// SimTime with different priorities; the priority-descending tie-break must still drain the
    /// higher-priority (upstream) predecessor first, so a downstream edge-triggered element
    /// never latches a transient produced by evaluating them in the wrong order.
    void testEqualPriorityTieBreakSettlesBeforeDownstream();

    /// A two-stage synchronous pipeline (both stages clocked from the same source, but with
    /// DIFFERENT nonzero propagation delays) must hold non-blocking commit semantics under real,
    /// elapsing temporal delay: the second flip-flop samples the first's PRE-edge value on the
    /// same clock edge that updates it, exactly matching real hardware pipeline behavior.
    void testTwoStagePipelineDifferentDelaysNonBlocking();

    /// A level-sensitive D-latch is transparent (delayed) while Enable is high and opaque while
    /// Enable is low — distinct from the edge-triggered flip-flop, verified under propagation delay.
    void testLatchTransparencyUnderDelay();

    /// An asynchronous active-low ~Clear forces Q low with no clock edge (delayed by the
    /// flip-flop's own propagation delay) and holds it there while asserted.
    void testAsyncClearUnderDelay();

    /// A JK flip-flop set/reset/toggles on delayed rising clock edges (its own captured-J/K
    /// state machine), each Q change appearing only after the propagation delay.
    void testJKFlipFlopUnderDelay();

    /// An SR flip-flop set/resets on delayed rising clock edges, including the S=R=1 case
    /// (this implementation drives both Q and ~Q Active).
    void testSRFlipFlopUnderDelay();

    /// A level-sensitive SR latch sets/resets/holds under delay; S=R=1 drives both outputs low.
    void testSRLatchUnderDelay();

    /// An asynchronous active-low ~Preset forces Q high with no clock edge (delayed) and holds
    /// it there while asserted — the set-side mirror of ~Clear.
    void testAsyncPresetUnderDelay();

    /// Reconvergent fanout with unequal path delays produces a transient output glitch (a static
    /// hazard) when the reconverging gate is faster than the path skew, and absorbs it otherwise.
    void testReconvergentFanoutHazard();

    /// A pulse wider than the gate delay propagates to the output (the transport complement to
    /// the inertial absorption of a narrow pulse).
    void testWidePulsePropagates();

    /// A zero-delay feedback loop simulated in temporal mode still collapses to one timestamp,
    /// hits the per-timestamp oscillation cap, and canonicalizes to Unknown.
    void testZeroDelayLoopOscillatesToUnknownInTemporalMode();

    /// A wireless Tx→Rx hop carries a timed signal: the element downstream of the Rx still
    /// incurs its propagation delay, proving wireless edges join the timed successor graph.
    void testWirelessHopUnderDelay();

    /// XOR and NOR gates produce correct, delayed outputs in temporal mode (breadth: same
    /// delay mechanism, different truth tables).
    void testXorNorUnderDelay();

    /// MUX and DEMUX route correctly and with propagation delay in temporal mode.
    void testMuxDemuxUnderDelay();

    /// Restarting mid-temporal-run resets sim time, re-seeds the whole network, and preserves
    /// temporal mode + the element's propagation delay so the second run is timed identically.
    void testRestartDuringTemporalRun();

    /// XNOR produces correct, delayed outputs in temporal mode (the last combinational gate).
    void testXnorUnderDelay();

    /// The model has no setup/hold window: Data changing in the same tick as the clock edge
    /// still yields a deterministic capture (never metastable/Unknown) — a deliberate omission.
    void testNoSetupHoldViolation();
};
