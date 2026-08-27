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
    void testGateDelay_data();
    void testGateDelay();

    /// Two chained NOTs accumulate their delays (second flips after delay1 + delay2).
    void testChainedDelay_data();
    void testChainedDelay();

    /// An input pulse narrower than the gate delay is absorbed (inertial delay model).
    void testGlitchAbsorbed_data();
    void testGlitchAbsorbed();

    /// A flip-flop clocked directly by another flip-flop's Q (a ripple/derived-clock chain,
    /// no gate in between) must react to the edge its predecessor produces, in both functional
    /// and temporal mode, on every edge — not just the first.
    void testRippleFlipFlopReevaluation_data();
    void testRippleFlipFlopReevaluation();

    /// A structural edit (element freed + Simulation::initialize()) while temporal events are
    /// still pending must drop those events: they hold raw pointers into the old netlist, so
    /// draining one afterwards would dereference freed memory.
    void testStructuralEditDropsPendingEvents();

    /// A beginTimedRun()/endTimedRun() bracket runs the enclosed manual update() loop on its
    /// own delayed timeline (the BeWavedDolphin sweep's contract) and hands the live session
    /// back its own tick window afterwards.
    void testTimedRunBracketDelaysInsideAndRestoresAfter();

    /// A bracket opened at 0 ns/tick settles within the tick even when the live session was
    /// left in temporal mode — a functional sweep must never inherit someone else's window.
    void testTimedRunZeroWindowIgnoresLiveTemporalMode();

    // --- Shipped temporal example circuits ---

    /// Examples/temporal_ring_oscillator.panda: Unknown in functional mode (no stable value),
    /// oscillating under delay without tripping the oscillation cap.
    void testExampleRingOscillator();

    /// One tick's work must not scale without bound with the sim-time window. The drain was
    /// limited only by targetTime, so event count grew linearly with m_timePerTick -- and the
    /// speed selector DEFAULTS to 1x (1,000,000 ns/tick). Measured on the shipped ring
    /// oscillator before the fix: 9 us at 1 ns/tick, 182 us at 100, 5,099 us at 10,000 and
    /// 541,461 us at 1,000,000 -- half a second of blocking work per 1 ms timer tick.
    void testTemporalTickIsBoundedRegardlessOfWindow();
    void testPropertyLagScalesWithTheDelay_data();
    void testPropertyLagScalesWithTheDelay();
    void testPropertyBothModesReachTheSameSteadyState_data();
    void testPropertyBothModesReachTheSameSteadyState();

    /// Examples/temporal_static_hazard.panda: the tuned delays survive the .panda round-trip,
    /// and F = A OR NOT A dips low after A falls — a static-1 hazard only delays can produce.
    void testExampleStaticHazard();

    /// Examples/temporal_gate_delay_chain.panda: an edge marches down the chain, so the last
    /// tap changes strictly later than the first.
    void testExampleGateDelayChain();

    // --- Metamorphic properties ---
    // Each pins a RELATIONSHIP that no fixed-parameter test can see: how a result must change
    // when a parameter does. They are the acceptance criteria for publish-side delay and phase
    // separation.

    /// Inertial delay: no element may emit an output run shorter than its own delay, so a
    /// pulse train far below the gate delay must not pass at all.
    void testPropertyNoOutputRunShorterThanDelay_data();
    void testPropertyNoOutputRunShorterThanDelay();
    /// The boundary the absorption property leaves open: a pulse EXACTLY as wide as the delay.
    /// It propagates -- inertial delay absorbs pulses strictly narrower than the delay. Pinned
    /// because an argument from the event comparator predicted the opposite and was wrong.
    void testPropertyPulseExactlyEqualToDelayPropagates_data();
    void testPropertyPulseExactlyEqualToDelayPropagates();

    /// The seed's first documented promise: it settles the whole network AT the current instant
    /// rather than spreading power-on settling over max-path-delay. Pinned here because
    /// processEvents() documents the choice but nothing else would notice a later switch to
    /// event-driven power-on settling.
    void testSeedSettlesWholeNetworkAtPowerOnEvenInTemporalMode();

    /// The seed's second promise: because power-on settling happens at the instant in BOTH
    /// modes, what a flip-flop captures at power-on must not depend on the simulation mode.
    void testPowerOnCaptureIsModeIndependent();

    /// The power-on / re-seed pass must use the same NON-BLOCKING semantics as the drain.
    /// Settling every element inline in priority order -- Memory included -- would let stage 2
    /// of a shift register read stage 1's just-committed Q instead of its pre-edge value, and
    /// the input would ripple through the whole register in one tick. Only visible with a real
    /// Clock:
    /// initialize() resets clocks HIGH while m_simLastClk starts Inactive, so the first tick
    /// after any rebuild presents a rising edge to every flip-flop at once. Tests that clock
    /// from an InputSwitch (which starts LOW) never see it.
    void testShiftRegisterShiftsOneStagePerEdgeFromPowerOn();

    /// Capture is invariant under data-arrival time: given D stable at the clock edge, Q must
    /// rise exactly one propagation delay later, whatever the data-to-clock gap.
    void testPropertyCaptureInvariantUnderDataArrival();

    /// The mirror of the property above: D is stable BEFORE the edge and then CHANGES after
    /// it, inside the flip-flop's own propagation delay. The capture already happened, so a
    /// later data change must not undo it -- and Q must still land at edge + delay, not be
    /// pushed out by the re-evaluation. Both depend on beginDeferredCommit() being idempotent
    /// while a window is open: re-seeding staging would overwrite the staged capture with the
    /// pre-edge output, and the superseding publish would commit that.
    void testPropertyCaptureSurvivesPostEdgeDataChange();

    /// A bystander gate's lag equals its own delay whether or not a flip-flop commits during
    /// the run.
    void testPropertyBystanderLagUnaffectedByCommit();

    /// A ripple hop's lag is the element's delay, invariant under the tick window.
    void testPropertyRippleLagInvariantUnderTickWindow();
};
