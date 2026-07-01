// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestUnifiedTimed.h"

#include <QtTest>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "App/Element/GraphicElements/Xor.h"
#include "App/Scene/Commands.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Common/TestUtils.h"

void TestUnifiedTimed::testGateDelay()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    // Functional baseline (zero delay): NOT(0) settles to 1 immediately.
    sw->setOn(false);
    sim->update();
    QCOMPARE(notGate->outputValue(0), Status::Active);

    // Switch to temporal: 5-unit gate delay, 1 unit of sim-time per update().
    sim->setElementDelay(notGate, 5);
    sim->setTimePerTick(1);

    const SimTime t0 = sim->currentTime();
    sw->setOn(true); // input rises; NOT must NOT fall until t0 + 5

    for (int i = 1; i <= 4; ++i) {
        sim->update();
        QVERIFY2(notGate->outputValue(0) == Status::Active,
                 qPrintable(QString("NOT changed too early at t=%1").arg(sim->currentTime())));
    }

    sim->update(); // reaches t0 + 5
    QCOMPARE(sim->currentTime(), t0 + 5);
    QCOMPARE(notGate->outputValue(0), Status::Inactive); // NOT(1) = 0, exactly after the delay
}

void TestUnifiedTimed::testChainedDelay()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *not1 = new Not();
    auto *not2 = new Not();
    builder.add(sw, not1, not2);
    builder.connect(sw, 0, not1, 0);
    builder.connect(not1, 0, not2, 0);
    Simulation *sim = builder.initSimulation();

    // Baseline: sw=0 -> not1=1 -> not2=0.
    sw->setOn(false);
    sim->update();
    QCOMPARE(not1->outputValue(0), Status::Active);
    QCOMPARE(not2->outputValue(0), Status::Inactive);

    sim->setElementDelay(not1, 3);
    sim->setElementDelay(not2, 3);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    sw->setOn(true); // at t=0: not1 flips at t=3, not2 flips at t=3+3=6

    advanceTo(2);
    QCOMPARE(not1->outputValue(0), Status::Active);   // delay 3 not reached
    advanceTo(3);
    QCOMPARE(not1->outputValue(0), Status::Inactive); // not1 flips at t=3
    advanceTo(5);
    QCOMPARE(not2->outputValue(0), Status::Inactive); // not2 delay (t=6) not reached
    advanceTo(6);
    QCOMPARE(not2->outputValue(0), Status::Active);    // not2 flips at t=6 (cumulative)
}

void TestUnifiedTimed::testGlitchAbsorbed()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    sw->setOn(false);
    sim->update();
    QCOMPARE(notGate->outputValue(0), Status::Active);

    sim->setElementDelay(notGate, 5);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    // Pulse the input high for 2 units — narrower than the 5-unit gate delay.
    sw->setOn(true);
    advanceTo(2);
    sw->setOn(false);

    // Inertial delay: when the scheduled re-evaluation fires (t=5 and t=7), the input has
    // already returned low, so NOT re-reads 0 -> stays Active. The glitch never appears.
    advanceTo(6);
    QCOMPARE(notGate->outputValue(0), Status::Active);
    advanceTo(10);
    QCOMPARE(notGate->outputValue(0), Status::Active);
}

void TestUnifiedTimed::testRippleFlipFlopReevaluation()
{
    // FF1's Q feeds FF2's Clock directly (no gate in between) — a ripple/derived-clock
    // topology. Regression test for a bug where a flip-flop's committed change never
    // propagated to a directly-fed downstream flip-flop: successor scheduling inside
    // processEvents() is gated on outputChanged(), which stays false for a deferred
    // (staged) Memory-group write, and the post-edge resettle pass explicitly skips
    // Memory-group elements so it never re-clocks them either. Without the fix, FF2 never
    // reacts to FF1's Q, in either mode, on any edge.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *swD1 = new InputSwitch();
    auto *swClk1 = new InputSwitch();
    auto *swD2 = new InputSwitch();
    auto *ff1 = new DFlipFlop();
    auto *ff2 = new DFlipFlop();
    builder.add(swD1, swClk1, swD2, ff1, ff2);
    builder.connect(swD1, 0, ff1, 0);   // D
    builder.connect(swClk1, 0, ff1, 1); // Clock
    builder.connect(ff1, 0, ff2, 1);    // FF1.Q -> FF2.Clock (ripple, no gate)
    builder.connect(swD2, 0, ff2, 0);   // D
    Simulation *sim = builder.initSimulation();

    swD1->setOn(true);
    swD2->setOn(true);
    swClk1->setOn(false);
    sim->update(); // baseline settle: no edge yet, both flip-flops stay at power-on default
    QCOMPARE(ff1->outputValue(0), Status::Inactive);
    QCOMPARE(ff2->outputValue(0), Status::Inactive);

    // --- Functional mode: FF2 must react to FF1's edge within the SAME update() call. ---
    swClk1->setOn(true);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Active);
    QCOMPARE(ff2->outputValue(0), Status::Active); // FF2 must capture D2 on FF1's rising edge

    // Drive FF1.Q back to Inactive (a FALLING transition on FF2's clock — FF2, edge-triggered
    // on rising only, must NOT recapture here) before producing a second, genuine rising edge.
    swD1->setOn(false);
    swClk1->setOn(false);
    sim->update();
    swClk1->setOn(true);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Inactive);
    QCOMPARE(ff2->outputValue(0), Status::Active); // unchanged: FF1's Q fell, not rose

    // A second genuine RISING edge on FF1.Q — FF2 must react to THIS one too, not just the
    // very first (proves this isn't a one-shot fluke of the initial full network seed).
    swD1->setOn(true);
    swD2->setOn(false);
    swClk1->setOn(false);
    sim->update();
    swClk1->setOn(true);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Active);
    QCOMPARE(ff2->outputValue(0), Status::Inactive); // FF2 must react to this edge too

    // --- Temporal mode: the same chain, with real, elapsing per-element delays. ---
    // Drive FF1.Q back to Inactive first (still functional mode) so the upcoming temporal
    // edge is a genuine rising transition, not a same-value re-assertion.
    swD1->setOn(false);
    swClk1->setOn(false);
    sim->update();
    swClk1->setOn(true);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Inactive);

    swClk1->setOn(false);
    swD1->setOn(true);
    swD2->setOn(true);
    sim->update();
    sim->setElementDelay(ff1, 5);
    sim->setElementDelay(ff2, 5);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    swClk1->setOn(true); // rising edge for FF1
    advanceTo(sim->currentTime() + 15);
    QCOMPARE(ff1->outputValue(0), Status::Active);
    QCOMPARE(ff2->outputValue(0), Status::Active); // FF2 must react once FF1 actually commits
}

void TestUnifiedTimed::testStructuralEditDropsPendingEvents()
{
    // initialize() must drop pending events. They are stale twice over: SimEvent::target is a
    // raw pointer into the old netlist (structural commands genuinely FREE elements — delete,
    // morph, split-undo), and the baked-in priority comes from the pre-edit topological sort.
    // In temporal mode a pending cross-tick event is easy to produce: give a gate a delay far
    // beyond the tick window. Pre-fix, draining past that timestamp after the edit dereferences
    // the freed element (deterministic use-after-free under ASan).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *doomed = new Not();
    auto *keep = new Not(); // survives the edit; proves the reseeded circuit still works
    builder.add(sw, doomed, keep);
    builder.connect(sw, 0, doomed, 0);
    builder.connect(sw, 0, keep, 0);
    Simulation *sim = builder.initSimulation();

    sw->setOn(false);
    sim->update(); // functional baseline settle
    QCOMPARE(doomed->outputValue(0), Status::Active);
    QCOMPARE(keep->outputValue(0), Status::Active);

    sim->setElementDelay(doomed, 100); // far beyond the 1-unit tick window below
    sim->setTimePerTick(1);
    sw->setOn(true);
    sim->update(); // doomed's re-evaluation is now pending ~100 units in the future

    // The real structural-edit path: DeleteItemsCommand::redo() severs the wire, FREES the
    // element, and re-initializes via setCircuitUpdateRequired().
    workspace.scene()->receiveCommand(
        new DeleteItemsCommand(QList<QGraphicsItem *>{doomed}, workspace.scene()));

    // Drain far past the stale event's timestamp: nothing may touch the freed element, and
    // the surviving gate settles from the post-edit reseed.
    for (int i = 0; i < 300; ++i) {
        sim->update();
    }
    QCOMPARE(keep->outputValue(0), Status::Inactive); // NOT(1), reseeded and settled
}

void TestUnifiedTimed::testRestartResetsRecorderTimeline()
{
    // Regression test: Simulation::restart() (and resetEventTracking()) reset the sim clock to
    // 0 but must also clear the waveform recorder's recorded transition history — otherwise a
    // transition recorded after the restart lands at a SMALLER timestamp than one already in
    // the trace, violating the ascending-time order SignalTrace::statusAt() (and the waveform
    // viewer's paint routine) both assume.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    sim->waveformRecorder().watchSignal("out", notGate, 0);
    sim->waveformRecorder().setRecording(true);
    sim->setTimePerTick(1);

    sw->setOn(false);
    sim->update();
    sw->setOn(true);
    sim->update();
    sw->setOn(false);
    sim->update();

    QVERIFY(!sim->waveformRecorder().trace(0).transitions.isEmpty());

    sim->restart();

    // Pre-fix: restart() reset m_currentTime to 0 but left the recorder's prior (larger-
    // timestamp) transitions in place — the trace stayed non-empty here, and the next
    // recorded transition (at a small post-restart timestamp) would have landed after it.
    QVERIFY(sim->waveformRecorder().trace(0).transitions.isEmpty());

    // Recording after the restart must produce a clean, monotonic trace of its own.
    sim->initialize();
    sim->update();
    sw->setOn(true);
    sim->update();
    QVERIFY(!sim->waveformRecorder().trace(0).transitions.isEmpty());
    const auto &transitions = sim->waveformRecorder().trace(0).transitions;
    for (int i = 1; i < transitions.size(); ++i) {
        QVERIFY(transitions.at(i).first >= transitions.at(i - 1).first);
    }
}

void TestUnifiedTimed::testSequentialTransitionRecordedAtCommitTime()
{
    // recordAll() runs inside processEvents(), always BEFORE commitDeferredOutputs()
    // publishes a flip-flop's staged Q — and a steady tick drains no events at all. Without
    // update()'s post-commit capture, Q's transition would be recorded only at the NEXT
    // event-bearing timestamp (the next clock edge — half a period late for a slow clock).
    // The recorded timestamp must be the tick boundary at which the commit published: at this
    // test's 1-unit tick resolution, exactly edge time + the flip-flop's delay.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *d = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ff = new DFlipFlop();
    builder.add(d, clk, ff);
    builder.connect(d, 0, ff, 0);   // Data
    builder.connect(clk, 0, ff, 1); // Clock
    Simulation *sim = builder.initSimulation();

    d->setOn(true);
    clk->setOn(false);
    sim->update(); // functional baseline: no edge yet, Q=0, Data=1 captured as pre-edge value
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    const SimTime delay = 10;
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("Q", ff, 0);
    recorder.setRecording(true);

    for (SimTime i = 0; i < 2 * delay; ++i) {
        sim->update(); // settle into temporal mode; no events, nothing recorded yet
    }

    const SimTime tEdge = sim->currentTime();
    clk->setOn(true); // rising edge: the flip-flop evaluates and commits at tEdge + delay

    // Run WELL past the edge with no further circuit activity. Pre-fix there is no later
    // event-bearing tick inside this window, so the rise would never be recorded here.
    for (SimTime i = 0; i < 5 * delay; ++i) {
        sim->update();
    }
    QCOMPARE(ff->outputValue(0), Status::Active); // the value itself is long since published

    const auto &transitions = recorder.trace(0).transitions;
    SimTime riseTime = SIM_TIME_UNSET;
    for (const auto &tr : transitions) {
        if (tr.second == Status::Active) {
            riseTime = tr.first;
            break;
        }
    }
    QVERIFY2(riseTime != SIM_TIME_UNSET, "Q's rise was never recorded");
    QCOMPARE(riseTime, tEdge + delay);
}

void TestUnifiedTimed::testFlipFlopClockedUnderDelay()
{
    // First temporal-mode test of sequential logic. A D-flip-flop with a propagation delay must
    // (a) ignore Data except on a rising clock edge, and (b) settle Q exactly one delay after that
    // edge — never immediately. This exercises the m_simLastValue delayed-capture trick under the
    // event scheduler (the flip-flop re-evaluates at clock-edge-time + its delay).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *d = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ff = new DFlipFlop();
    builder.add(d, clk, ff);
    builder.connect(d, 0, ff, 0);   // Data
    builder.connect(clk, 0, ff, 1); // Clock
    Simulation *sim = builder.initSimulation();

    // Functional baseline: Data=1, Clock=0 ⇒ no edge ⇒ Q stays 0.
    d->setOn(true);
    clk->setOn(false);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    // Temporal: 10-unit flip-flop delay, 1 unit per update().
    const SimTime delay = 10;
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    // Let the flip-flop re-evaluate once with Data=1 stable so its captured Data is 1, then assert
    // it has NOT clocked (Clock is still low — Q must stay 0 no matter how long we wait).
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    // Rising clock edge: Q must remain 0 well past the edge but before the delay elapses...
    const SimTime tEdge = sim->currentTime();
    clk->setOn(true);
    advanceTo(tEdge + 2);
    QVERIFY2(ff->outputValue(0) == Status::Inactive, "Q changed immediately — delay not applied");
    advanceTo(tEdge + (delay - 2));
    QVERIFY2(ff->outputValue(0) == Status::Inactive, "Q changed before the propagation delay");

    // ...and become 1 once the delay has elapsed (captured Data=1 on the rising edge).
    advanceTo(tEdge + delay + 3);
    QCOMPARE(ff->outputValue(0), Status::Active);

    // A second edge with Data=0 must drive Q back to 0 (again only after the delay).
    d->setOn(false);
    clk->setOn(false);
    advanceTo(sim->currentTime() + 2 * delay); // settle the new Data; Clock low ⇒ Q unchanged
    QCOMPARE(ff->outputValue(0), Status::Active);

    const SimTime tEdge2 = sim->currentTime();
    clk->setOn(true);
    advanceTo(tEdge2 + 2);
    QCOMPARE(ff->outputValue(0), Status::Active); // not yet
    advanceTo(tEdge2 + delay + 3);
    QCOMPARE(ff->outputValue(0), Status::Inactive); // captured Data=0
}

void TestUnifiedTimed::testCounterBitTogglesUnderDelay()
{
    // A T-flip-flop (T=1) is the building block of a ripple counter: it must toggle Q exactly once
    // per rising clock edge — not twice — even when its output change is deferred by a propagation
    // delay. (A double-toggle would leave Q unchanged after an edge and break every counter.)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *t = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ff = new TFlipFlop();
    builder.add(t, clk, ff);
    builder.connect(t, 0, ff, 0);   // T
    builder.connect(clk, 0, ff, 1); // Clock
    Simulation *sim = builder.initSimulation();

    // Functional baseline: T=1, Clock=0 ⇒ Q=0.
    t->setOn(true);
    clk->setOn(false);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    const SimTime delay = 8;
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    // One clean clock pulse: rise (toggle expected after the delay), then fall.
    const auto pulseClock = [&]() {
        clk->setOn(true);
        advanceTo(sim->currentTime() + delay + 4); // let the edge clock through the delay
        clk->setOn(false);
        advanceTo(sim->currentTime() + delay + 4); // falling edge does nothing
    };

    advanceTo(sim->currentTime() + 2 * delay); // capture T=1 stably first

    pulseClock();
    QCOMPARE(ff->outputValue(0), Status::Active);   // toggled 0 -> 1 once

    pulseClock();
    QCOMPARE(ff->outputValue(0), Status::Inactive); // toggled 1 -> 0 once (not back to 1)

    pulseClock();
    QCOMPARE(ff->outputValue(0), Status::Active);   // and once more
}

void TestUnifiedTimed::testRingOscillatorTemporal()
{
    // A 3-inverter ring has no stable state. At zero delay (functional mode) the engine caps the
    // per-timestamp delta cycles and canonicalizes the loop to Unknown. WITH per-gate delays the
    // toggles spread across future timestamps, so the ring is a real oscillator that never hits the
    // cap — its watched node must swing cleanly between Active and Inactive over time.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *n1 = new Not();
    auto *n2 = new Not();
    auto *n3 = new Not();
    builder.add(n1, n2, n3);
    builder.connect(n1, 0, n2, 0);
    builder.connect(n2, 0, n3, 0);
    builder.connect(n3, 0, n1, 0); // close the loop
    Simulation *sim = builder.initSimulation();

    // Go temporal BEFORE the first settle (a functional settle would freeze the ring at Unknown,
    // and NOT(Unknown)=Unknown is a fixed point it could never leave).
    sim->setTimePerTick(1);
    sim->setElementDelay(n1, 5);
    sim->setElementDelay(n2, 5);
    sim->setElementDelay(n3, 5);

    int transitions = 0;
    bool sawUnknown = false;
    Status prev = n1->outputValue(0);
    for (int i = 0; i < 150; ++i) { // 150 ns ≫ the 30 ns oscillation period
        sim->update();
        const Status now = n1->outputValue(0);
        if (now == Status::Unknown) {
            sawUnknown = true;
        }
        if (now != prev) {
            ++transitions;
            prev = now;
        }
    }

    QVERIFY2(!sawUnknown, "Temporal ring hit the oscillation cap and went Unknown");
    QVERIFY2(transitions >= 3, qPrintable(
        QString("Ring did not oscillate: only %1 transitions in 150 ns").arg(transitions)));
}

void TestUnifiedTimed::testGatedClockNoGlitchTemporal()
{
    // Gated clock: gated = master-clock AND enable, feeding a T-flip-flop (T=1). The
    // (time, priority-desc) drain must settle the AND before the flip-flop samples it, so the
    // flip-flop toggles exactly once per genuine gated rising edge and not at all while the clock
    // is gated off. Delays on the AND and flip-flop are equal so their edges collide at a single
    // timestamp — the case the priority ordering exists to handle.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *clk = new InputSwitch();
    auto *en = new InputSwitch();
    auto *t = new InputSwitch();
    auto *gated = new And();
    auto *ff = new TFlipFlop();
    builder.add(clk, en, t, gated, ff);
    builder.connect(clk, 0, gated, 0);
    builder.connect(en, 0, gated, 1);
    builder.connect(gated, 0, ff, 1); // gated clock → Clock
    builder.connect(t, 0, ff, 0);     // T = 1
    Simulation *sim = builder.initSimulation();

    // Functional baseline: T=1, enable=0, clock=0 ⇒ gated=0, Q=0.
    t->setOn(true);
    en->setOn(false);
    clk->setOn(false);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    const SimTime delay = 6;
    sim->setElementDelay(gated, delay);
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    const auto pulseClock = [&]() {
        clk->setOn(true);
        advanceTo(sim->currentTime() + 2 * delay + 4);
        clk->setOn(false);
        advanceTo(sim->currentTime() + 2 * delay + 4);
    };

    advanceTo(sim->currentTime() + 2 * delay); // settle T=1

    // Enable LOW: clock pulses are gated out — the flip-flop must never toggle.
    pulseClock();
    pulseClock();
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    // Enable HIGH: each clock pulse is a genuine gated rising edge → toggle exactly once.
    en->setOn(true);
    advanceTo(sim->currentTime() + 2 * delay); // settle the enable edge (clock still low ⇒ no edge)
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    pulseClock();
    QCOMPARE(ff->outputValue(0), Status::Active);   // one toggle
    pulseClock();
    QCOMPARE(ff->outputValue(0), Status::Inactive); // one more toggle — never a glitchy double
}

void TestUnifiedTimed::testEqualPriorityTieBreakSettlesBeforeDownstream()
{
    // Two independent branches off the same clk, both with a cumulative 6 ns delay, land their
    // "settle" events at the identical SimTime with DIFFERENT priorities:
    //   clk -> not1(3ns) -> not2(3ns)   [priority 3, cumulative 6ns]
    //   clk -> not3(6ns)                [priority 3, same 6ns]
    // xorL = Xor(not2, not3) (0ns, priority 2) settles to the tautology clk XOR ~clk == Active,
    // since not2 tracks clk and not3 tracks ~clk. A correct priority-descending tie-break always
    // drains BOTH same-time predecessors (priority 3) before xorL (priority 2) re-fires, so xorL
    // never observably changes. A broken tie-break could let xorL fire on a stale input, producing
    // a real (if transient) glitch — which a downstream EDGE-TRIGGERED element latches permanently
    // (unlike pure combinational logic, which self-heals within the same timestamp bucket).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *clk = new InputSwitch();
    auto *not1 = new Not();
    auto *not2 = new Not();
    auto *not3 = new Not();
    auto *xorL = new Xor();
    auto *tOn = new InputSwitch();
    auto *observerFF = new TFlipFlop();
    builder.add(clk, not1, not2, not3, xorL, tOn, observerFF);
    builder.connect(clk, 0, not1, 0);
    builder.connect(not1, 0, not2, 0);
    builder.connect(clk, 0, not3, 0);
    builder.connect(not2, 0, xorL, 0);
    builder.connect(not3, 0, xorL, 1);
    builder.connect(tOn, 0, observerFF, 0);  // T
    builder.connect(xorL, 0, observerFF, 1); // Clock
    Simulation *sim = builder.initSimulation();

    tOn->setOn(true);
    clk->setOn(false);
    sim->update(); // baseline settle

    sim->setElementDelay(not1, 3);
    sim->setElementDelay(not2, 3);
    sim->setElementDelay(not3, 6);
    sim->setElementDelay(xorL, 0);
    // Zero delay on the observer itself is essential: any positive delay here would inertially
    // absorb a transient glitch (exactly like testGlitchAbsorbed's gate-level absorption) before
    // it could register as an edge, masking a broken tie-break instead of exposing it.
    sim->setElementDelay(observerFF, 0);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    const Status q0 = observerFF->outputValue(0);

    // Toggle clk several times, well past the 6 ns cumulative delay each time. If the
    // priority tie-break were broken, xorL would glitch off the tautology and observerFF
    // (edge-triggered, T held Active) would toggle permanently on the very first edge.
    for (int i = 0; i < 3; ++i) {
        clk->setOn(!clk->isOn());
        advanceTo(sim->currentTime() + 15);
        QCOMPARE(observerFF->outputValue(0), q0);
    }
}

void TestUnifiedTimed::testTwoStagePipelineDifferentDelaysNonBlocking()
{
    // A genuine two-stage synchronous pipeline (NOT the ripple/derived-clock topology covered by
    // testRippleFlipFlopReevaluation): both flip-flops share the SAME external clock, with a gate
    // between them, and DIFFERENT nonzero propagation delays (ff2 smaller than ff1's). Because
    // Simulation's non-blocking commit is scoped per update() TICK (not per clock edge), ff2's
    // smaller delay means it fires — and correctly samples ff1's PRE-edge Q through inv — many
    // ticks before ff1 itself finally commits. This is the textbook pipeline result and holds
    // regardless of any same-tick tie-break subtlety, since there's no coincidence to race.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *d0 = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ff1 = new DFlipFlop();
    auto *inv = new Not();
    auto *ff2 = new DFlipFlop();
    builder.add(d0, clk, ff1, inv, ff2);
    builder.connect(d0, 0, ff1, 0);   // Data
    builder.connect(clk, 0, ff1, 1);  // Clock
    builder.connect(ff1, 0, inv, 0);  // Q -> gate
    builder.connect(inv, 0, ff2, 0);  // Data
    builder.connect(clk, 0, ff2, 1);  // SAME shared Clock, not derived from ff1
    Simulation *sim = builder.initSimulation();

    d0->setOn(true);
    clk->setOn(false);
    sim->update();
    QCOMPARE(ff1->outputValue(0), Status::Inactive);
    QCOMPARE(ff2->outputValue(0), Status::Inactive);

    sim->setElementDelay(ff1, 20);
    sim->setElementDelay(ff2, 8); // deliberately smaller than ff1's delay
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    // --- Edge 1 ---
    advanceTo(sim->currentTime() + 25); // prime, no edge yet
    const SimTime t0 = sim->currentTime();
    clk->setOn(true);
    advanceTo(t0 + 8);
    QCOMPARE(ff2->outputValue(0), Status::Active);   // NOT(ff1's OLD Q=Inactive)
    QCOMPARE(ff1->outputValue(0), Status::Inactive); // ff1's own 20ns delay not yet elapsed
    advanceTo(t0 + 20);
    QCOMPARE(ff1->outputValue(0), Status::Active);   // ff1 finally captures D=Active
    QCOMPARE(ff2->outputValue(0), Status::Active);   // unchanged: ff2 already committed this edge

    // --- Edge 2 (new data, proving this holds across ticks, not just once) ---
    clk->setOn(false);
    advanceTo(sim->currentTime() + 25);
    d0->setOn(false);
    advanceTo(sim->currentTime() + 25);

    const SimTime t1 = sim->currentTime();
    clk->setOn(true);
    advanceTo(t1 + 8);
    QCOMPARE(ff2->outputValue(0), Status::Inactive); // NOT(ff1's OLD Q=Active, from edge 1)
    QCOMPARE(ff1->outputValue(0), Status::Active);   // ff1 unchanged so far
    advanceTo(t1 + 20);
    QCOMPARE(ff1->outputValue(0), Status::Inactive); // ff1 finally captures D=Inactive
    QCOMPARE(ff2->outputValue(0), Status::Inactive); // unchanged: already committed this edge
}
