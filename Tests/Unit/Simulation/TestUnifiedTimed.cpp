// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestUnifiedTimed.h"

#include <QtTest>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Nor.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "App/Element/GraphicElements/Xnor.h"
#include "App/Element/GraphicElements/Xor.h"
#include "App/Scene/Commands.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/WaveformRecorder.h"
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

void TestUnifiedTimed::testLatchTransparencyUnderDelay()
{
    // A D-latch is level-sensitive, not edge-triggered: Q tracks Data (after the latch delay)
    // while Enable is HIGH, and freezes the last value while Enable is LOW. This is the
    // complementary case to the flip-flop tests — same delay machinery, transparent semantics.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *data = new InputSwitch();
    auto *enable = new InputSwitch();
    auto *latch = new DLatch();
    builder.add(data, enable, latch);
    builder.connect(data, 0, latch, 0);   // Data
    builder.connect(enable, 0, latch, 1); // Enable
    Simulation *sim = builder.initSimulation();

    // Functional baseline: Enable=1, Data=0 ⇒ Q transparent = 0.
    enable->setOn(true);
    data->setOn(false);
    sim->update();
    QCOMPARE(latch->outputValue(0), Status::Inactive);

    const SimTime delay = 8;
    sim->setElementDelay(latch, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    // Enable HIGH: Data 0->1 is passed through, but only after the latch delay.
    const SimTime tData = sim->currentTime();
    data->setOn(true);
    advanceTo(tData + (delay - 2));
    QVERIFY2(latch->outputValue(0) == Status::Inactive, "Latch passed Data before its delay");
    advanceTo(tData + delay + 3);
    QCOMPARE(latch->outputValue(0), Status::Active); // transparent (delayed)

    // Enable LOW: the latch becomes opaque — Q must hold 1 no matter what Data does.
    enable->setOn(false);
    advanceTo(sim->currentTime() + 2 * delay); // settle the Enable edge
    data->setOn(false);
    advanceTo(sim->currentTime() + 4 * delay); // give Data plenty of time to (not) propagate
    QCOMPARE(latch->outputValue(0), Status::Active); // held — level-sensitive opacity

    // Enable HIGH again: Q becomes transparent and follows the now-low Data (after the delay).
    enable->setOn(true);
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(latch->outputValue(0), Status::Inactive);
}

void TestUnifiedTimed::testAsyncClearUnderDelay()
{
    // The active-low ~Clear is asynchronous: it forces Q low without a clock edge. Under temporal
    // mode the override still takes the flip-flop's own propagation delay to appear (the flip-flop
    // re-evaluates delay-units after ~Clear changes), and holds Q low while asserted.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *data = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *nClear = new InputSwitch();
    auto *ff = new DFlipFlop();
    builder.add(data, clk, nClear, ff);
    builder.connect(data, 0, ff, 0);   // Data
    builder.connect(clk, 0, ff, 1);    // Clock
    builder.connect(nClear, 0, ff, 3); // ~Clear (active low)
    Simulation *sim = builder.initSimulation();

    // Functional baseline: ~Clear deasserted (high), clock a 1 into Q.
    nClear->setOn(true);
    data->setOn(true);
    clk->setOn(false);
    sim->update();
    clk->setOn(true); // rising edge captures Data=1
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Active);

    const SimTime delay = 10;
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    advanceTo(sim->currentTime() + 2 * delay); // settle into temporal mode; Q still 1
    QCOMPARE(ff->outputValue(0), Status::Active);

    // Assert ~Clear with NO clock edge: Q must drop to 0, but only after the flip-flop delay.
    const SimTime tClear = sim->currentTime();
    nClear->setOn(false);
    advanceTo(tClear + 2);
    QVERIFY2(ff->outputValue(0) == Status::Active, "~Clear took effect immediately — delay ignored");
    advanceTo(tClear + delay + 3);
    QCOMPARE(ff->outputValue(0), Status::Inactive); // async-cleared (delayed)

    // While ~Clear is held low, Q stays 0 even if Data is high and a clock edge arrives.
    data->setOn(true);
    clk->setOn(false);
    advanceTo(sim->currentTime() + 2 * delay);
    clk->setOn(true);
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(ff->outputValue(0), Status::Inactive); // clear dominates

    // Release ~Clear: Q stays 0 until the next genuine rising edge re-captures Data.
    nClear->setOn(true);
    clk->setOn(false);
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(ff->outputValue(0), Status::Inactive); // no edge yet
    clk->setOn(true);
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(ff->outputValue(0), Status::Active); // edge re-captures Data=1
}

void TestUnifiedTimed::testJKFlipFlopUnderDelay()
{
    // The JK flip-flop has its own captured-J/K state machine (independent of D/T): on a rising
    // edge J=1,K=0 sets, J=0,K=1 resets, J=K=1 toggles. Verify each across delayed edges — Q must
    // change only after the flip-flop's propagation delay, and from the J/K sampled before it.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *j = new InputSwitch();
    auto *k = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ff = new JKFlipFlop();
    builder.add(j, k, clk, ff);
    builder.connect(j, 0, ff, 0);   // J
    builder.connect(clk, 0, ff, 1); // Clock
    builder.connect(k, 0, ff, 2);   // K
    Simulation *sim = builder.initSimulation();

    // Functional baseline: reset to a known Q=0 (J=0,K=1, one clock).
    j->setOn(false);
    k->setOn(true);
    clk->setOn(false);
    sim->update();
    clk->setOn(true);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    const SimTime delay = 10;
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    // Drive J/K to the next operation, settle so they are captured, then pulse one delayed edge.
    const auto clockOnce = [&](bool jv, bool kv) {
        clk->setOn(false);
        j->setOn(jv);
        k->setOn(kv);
        advanceTo(sim->currentTime() + 2 * delay); // capture J/K, no edge
        clk->setOn(true);                           // rising edge → acts after the delay
    };

    // Set: J=1,K=0 ⇒ Q=1, only after the delay.
    clockOnce(true, false);
    const SimTime tSet = sim->currentTime();
    advanceTo(tSet + 2);
    QVERIFY2(ff->outputValue(0) == Status::Inactive, "JK set took effect before the delay");
    advanceTo(tSet + delay + 3);
    QCOMPARE(ff->outputValue(0), Status::Active);

    // Reset: J=0,K=1 ⇒ Q=0.
    clockOnce(false, true);
    advanceTo(sim->currentTime() + delay + 4);
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    // Toggle: J=K=1 ⇒ Q flips on each edge.
    clockOnce(true, true);
    advanceTo(sim->currentTime() + delay + 4);
    QCOMPARE(ff->outputValue(0), Status::Active);
    clockOnce(true, true);
    advanceTo(sim->currentTime() + delay + 4);
    QCOMPARE(ff->outputValue(0), Status::Inactive);
}

void TestUnifiedTimed::testSRFlipFlopUnderDelay()
{
    // The clocked SR flip-flop captures S/R on a rising edge: S=1,R=0 sets, S=0,R=1 resets, and
    // (in this implementation) S=R=1 drives both Q and ~Q Active. Verify each under delay.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *s = new InputSwitch();
    auto *r = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ff = new SRFlipFlop();
    builder.add(s, r, clk, ff);
    builder.connect(s, 0, ff, 0);   // S
    builder.connect(clk, 0, ff, 1); // Clock
    builder.connect(r, 0, ff, 2);   // R
    Simulation *sim = builder.initSimulation();

    // Functional baseline: reset to Q=0.
    s->setOn(false);
    r->setOn(true);
    clk->setOn(false);
    sim->update();
    clk->setOn(true);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    const SimTime delay = 10;
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    const auto clockOnce = [&](bool sv, bool rv) {
        clk->setOn(false);
        s->setOn(sv);
        r->setOn(rv);
        advanceTo(sim->currentTime() + 2 * delay);
        clk->setOn(true);
    };

    // Set: S=1,R=0 ⇒ Q=1, only after the delay.
    clockOnce(true, false);
    const SimTime tSet = sim->currentTime();
    advanceTo(tSet + 2);
    QVERIFY2(ff->outputValue(0) == Status::Inactive, "SR set took effect before the delay");
    advanceTo(tSet + delay + 3);
    QCOMPARE(ff->outputValue(0), Status::Active);

    // Reset: S=0,R=1 ⇒ Q=0.
    clockOnce(false, true);
    advanceTo(sim->currentTime() + delay + 4);
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    // S=R=1 case: this implementation drives both outputs Active.
    clockOnce(true, true);
    advanceTo(sim->currentTime() + delay + 4);
    QCOMPARE(ff->outputValue(0), Status::Active);
    QCOMPARE(ff->outputValue(1), Status::Active);
}

void TestUnifiedTimed::testSRLatchUnderDelay()
{
    // A level-sensitive SR latch (no clock): Set=1 ⇒ Q=1, Reset=1 ⇒ Q=0, S=R=0 ⇒ hold,
    // S=R=1 ⇒ both outputs Inactive. Verify the delayed timing and the hold behaviour.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *set = new InputSwitch();
    auto *reset = new InputSwitch();
    auto *latch = new SRLatch();
    builder.add(set, reset, latch);
    builder.connect(set, 0, latch, 0);   // Set
    builder.connect(reset, 0, latch, 1); // Reset
    Simulation *sim = builder.initSimulation();

    // Functional baseline: S=R=0 ⇒ hold (initial Q=0).
    set->setOn(false);
    reset->setOn(false);
    sim->update();
    QCOMPARE(latch->outputValue(0), Status::Inactive);

    const SimTime delay = 8;
    sim->setElementDelay(latch, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    // Set=1 ⇒ Q=1, only after the delay.
    const SimTime tSet = sim->currentTime();
    set->setOn(true);
    advanceTo(tSet + (delay - 2));
    QVERIFY2(latch->outputValue(0) == Status::Inactive, "SR latch set before the delay");
    advanceTo(tSet + delay + 3);
    QCOMPARE(latch->outputValue(0), Status::Active);

    // Drop Set (S=R=0) ⇒ Q holds 1.
    set->setOn(false);
    advanceTo(sim->currentTime() + 3 * delay);
    QCOMPARE(latch->outputValue(0), Status::Active);

    // Reset=1 ⇒ Q=0, after the delay.
    const SimTime tReset = sim->currentTime();
    reset->setOn(true);
    advanceTo(tReset + (delay - 2));
    QVERIFY2(latch->outputValue(0) == Status::Active, "SR latch reset before the delay");
    advanceTo(tReset + delay + 3);
    QCOMPARE(latch->outputValue(0), Status::Inactive);

    // S=R=1 ⇒ both outputs Inactive.
    set->setOn(true);
    reset->setOn(true);
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(latch->outputValue(0), Status::Inactive);
    QCOMPARE(latch->outputValue(1), Status::Inactive);
}

void TestUnifiedTimed::testAsyncPresetUnderDelay()
{
    // The active-low ~Preset is the set-side mirror of ~Clear: it forces Q high without a clock
    // edge, delayed by the flip-flop's own propagation delay, and dominates while asserted.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *data = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *nPreset = new InputSwitch();
    auto *ff = new DFlipFlop();
    builder.add(data, clk, nPreset, ff);
    builder.connect(data, 0, ff, 0);    // Data
    builder.connect(clk, 0, ff, 1);     // Clock
    builder.connect(nPreset, 0, ff, 2); // ~Preset (active low)
    Simulation *sim = builder.initSimulation();

    // Functional baseline: ~Preset deasserted (high), clock a 0 into Q.
    nPreset->setOn(true);
    data->setOn(false);
    clk->setOn(false);
    sim->update();
    clk->setOn(true); // rising edge captures Data=0
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    const SimTime delay = 10;
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    // Assert ~Preset with no clock edge: Q rises to 1, only after the flip-flop delay.
    const SimTime tPreset = sim->currentTime();
    nPreset->setOn(false);
    advanceTo(tPreset + 2);
    QVERIFY2(ff->outputValue(0) == Status::Inactive, "~Preset took effect immediately — delay ignored");
    advanceTo(tPreset + delay + 3);
    QCOMPARE(ff->outputValue(0), Status::Active); // async-preset (delayed)

    // While ~Preset is held low, Q stays 1 even if Data=0 and a clock edge arrives.
    data->setOn(false);
    clk->setOn(false);
    advanceTo(sim->currentTime() + 2 * delay);
    clk->setOn(true);
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(ff->outputValue(0), Status::Active); // preset dominates

    // Release ~Preset: Q stays 1 until the next genuine rising edge recaptures Data=0.
    nPreset->setOn(true);
    clk->setOn(false);
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(ff->outputValue(0), Status::Active); // no edge yet
    clk->setOn(true);
    advanceTo(sim->currentTime() + 2 * delay);
    QCOMPARE(ff->outputValue(0), Status::Inactive); // edge recaptures Data=0
}

void TestUnifiedTimed::testReconvergentFanoutHazard()
{
    // F = A OR ~A is a tautology (always 1), but reconvergent fanout with unequal path delays
    // makes it glitch: when A falls, the direct path reaches the OR before the inverted path, so
    // the OR momentarily sees (0,0)=0 — a static-1 hazard. The transient survives only when the
    // OR is FASTER than the path skew (the NOT delay); otherwise the OR re-evaluates after ~A has
    // already risen and the glitch is inertially absorbed. Run both regimes.
    const auto glitchSeen = [](SimTime notDelay, SimTime orDelay) -> bool {
        WorkSpace workspace;
        CircuitBuilder builder(workspace.scene());
        auto *a = new InputSwitch();
        auto *notGate = new Not();
        auto *orGate = new Or();
        builder.add(a, notGate, orGate);
        builder.connect(a, 0, orGate, 0);   // direct path A → OR.in0
        builder.connect(a, 0, notGate, 0);  // inverted path A → NOT
        builder.connect(notGate, 0, orGate, 1); // → OR.in1
        Simulation *sim = builder.initSimulation();

        sim->setTimePerTick(100); // one wide tick drains the whole transient
        sim->setElementDelay(notGate, notDelay);
        sim->setElementDelay(orGate, orDelay);

        a->setOn(true);
        sim->update(); // settle F = 1 (A=1 ⇒ OR(1,0)=1)
        Q_ASSERT(orGate->outputValue(0) == Status::Active);

        auto &recorder = sim->waveformRecorder();
        recorder.watchSignal("F", orGate, 0);
        recorder.setRecording(true);

        a->setOn(false); // A: 1 → 0, the hazardous transition
        sim->update();   // drain the transient within one tick

        const auto &transitions = recorder.trace(0).transitions;
        bool sawLow = false;
        for (const auto &tr : transitions) {
            if (tr.second == Status::Inactive) {
                sawLow = true;
            }
        }
        // The output must settle back to 1 either way (tautology).
        Q_ASSERT(orGate->outputValue(0) == Status::Active);
        return sawLow;
    };

    // OR (delay 3) faster than the skew (NOT delay 10): the static-1 hazard propagates.
    QVERIFY2(glitchSeen(10, 3), "Reconvergent-fanout hazard glitch was not produced");

    // OR (delay 10) slower than the skew (NOT delay 3): the glitch is inertially absorbed.
    QVERIFY2(!glitchSeen(3, 10), "Hazard should have been absorbed when the gate is slower than the skew");
}

void TestUnifiedTimed::testWidePulsePropagates()
{
    // Inertial delay absorbs pulses NARROWER than the gate delay (testGlitchAbsorbed). The
    // complement: a pulse WIDER than the delay must propagate. A 10-unit input pulse through a
    // 5-unit NOT appears at the output as a (delayed) 10-unit pulse.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *a = new InputSwitch();
    auto *notGate = new Not();
    builder.add(a, notGate);
    builder.connect(a, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    a->setOn(false);
    sim->update();
    QCOMPARE(notGate->outputValue(0), Status::Active); // ~0 = 1

    sim->setElementDelay(notGate, 5);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    // Pulse A high for 10 units — wider than the 5-unit delay, so it must reach the output.
    const SimTime tRise = sim->currentTime();
    a->setOn(true);
    advanceTo(tRise + 8); // 8 > 5: the inverted pulse has propagated
    QVERIFY2(notGate->outputValue(0) == Status::Inactive, "Wide pulse was wrongly absorbed");

    a->setOn(false); // end the pulse (total high width ≈ 10 > delay)
    advanceTo(sim->currentTime() + 8);
    QCOMPARE(notGate->outputValue(0), Status::Active); // output pulse ended — back to ~0 = 1
}

void TestUnifiedTimed::testZeroDelayLoopOscillatesToUnknownInTemporalMode()
{
    // The per-timestamp oscillation cap is mode-independent. A 3-inverter ring whose gates are
    // forced to ZERO delay while running in temporal mode collapses all its toggles into a single
    // timestamp (exactly like functional mode), trips the cap, and canonicalizes to Unknown —
    // the complement of testRingOscillatorTemporal, where non-zero delays spread it over time.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *n1 = new Not();
    auto *n2 = new Not();
    auto *n3 = new Not();
    builder.add(n1, n2, n3);
    builder.connect(n1, 0, n2, 0);
    builder.connect(n2, 0, n3, 0);
    builder.connect(n3, 0, n1, 0);
    Simulation *sim = builder.initSimulation();

    sim->setTimePerTick(10);      // temporal mode...
    sim->setElementDelay(n1, 0);  // ...but a zero-delay loop, so events share one timestamp
    sim->setElementDelay(n2, 0);
    sim->setElementDelay(n3, 0);

    sim->update();

    QVERIFY2(sim->isInFeedbackLoop(n1), "Ring node should be detected as a feedback loop");
    QCOMPARE(n1->outputValue(0), Status::Unknown); // cap fired, canonicalized to Unknown
}

void TestUnifiedTimed::testWirelessHopUnderDelay()
{
    // Wireless Tx→Rx edges are spliced into the successor graph by a separate path
    // (connectWirelessElements). Verify the timed drain still treats them as real edges: a NOT
    // placed downstream of the Rx incurs its propagation delay exactly as it would on a wire.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *notGate = new Not();
    QVERIFY(tx && rx);
    tx->setLabel("SIG");
    tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("SIG");
    rx->setWirelessMode(WirelessMode::Rx);
    builder.add(sw, tx, rx, notGate);
    builder.connect(sw, 0, tx, 0);      // physical wire into Tx
    builder.connect(rx, 0, notGate, 0); // Rx output → NOT (wireless hop is sw → … → notGate)
    Simulation *sim = builder.initSimulation();

    // Baseline: sw=0 ⇒ (wirelessly) NOT input 0 ⇒ NOT out = 1.
    sw->setOn(false);
    sim->update();
    QCOMPARE(notGate->outputValue(0), Status::Active);

    const SimTime delay = 5;
    sim->setElementDelay(notGate, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };

    // Toggle the switch: the signal crosses the wireless hop (Node delay 0) and the downstream
    // NOT flips only after ITS delay — so the wireless edge is genuinely in the timed graph.
    const SimTime tEdge = sim->currentTime();
    sw->setOn(true);
    advanceTo(tEdge + 2);
    QVERIFY2(notGate->outputValue(0) == Status::Active, "NOT flipped before its delay across the wireless hop");
    advanceTo(tEdge + delay + 3);
    QCOMPARE(notGate->outputValue(0), Status::Inactive); // ~1 = 0, after the delay
}

void TestUnifiedTimed::testXorNorUnderDelay()
{
    // Breadth: XOR and NOR have their own updateLogic (different truth tables) but inherit the
    // element-agnostic delay scheduling. Verify correct, delayed outputs in temporal mode.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *a = new InputSwitch();
    auto *b = new InputSwitch();
    auto *xorGate = new Xor();
    auto *norGate = new Nor();
    builder.add(a, b, xorGate, norGate);
    builder.connect(a, 0, xorGate, 0);
    builder.connect(b, 0, xorGate, 1);
    builder.connect(a, 0, norGate, 0);
    builder.connect(b, 0, norGate, 1);
    Simulation *sim = builder.initSimulation();

    sim->setTimePerTick(100); // wide ticks settle each combination
    const SimTime delay = 7;
    a->setOn(false);
    b->setOn(false);
    sim->update();
    sim->setElementDelay(xorGate, delay);
    sim->setElementDelay(norGate, delay);

    const auto settle = [&](bool av, bool bv) {
        a->setOn(av);
        b->setOn(bv);
        sim->update();
    };

    settle(true, false); // (1,0): XOR=1, NOR=0
    QCOMPARE(xorGate->outputValue(0), Status::Active);
    QCOMPARE(norGate->outputValue(0), Status::Inactive);

    settle(true, true);  // (1,1): XOR=0, NOR=0
    QCOMPARE(xorGate->outputValue(0), Status::Inactive);
    QCOMPARE(norGate->outputValue(0), Status::Inactive);

    settle(false, false); // (0,0): XOR=0, NOR=1
    QCOMPARE(xorGate->outputValue(0), Status::Inactive);
    QCOMPARE(norGate->outputValue(0), Status::Active);

    // Confirm the delay actually applies to these element types: with fine ticks, an input
    // change must not flip XOR before its 7-unit delay.
    sim->setTimePerTick(1);
    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    const SimTime tEdge = sim->currentTime();
    a->setOn(true); // (1,0) ⇒ XOR should become 1, but only after the delay
    advanceTo(tEdge + 3);
    QVERIFY2(xorGate->outputValue(0) == Status::Inactive, "XOR flipped before its delay");
    advanceTo(tEdge + delay + 3);
    QCOMPARE(xorGate->outputValue(0), Status::Active);
}

void TestUnifiedTimed::testMuxDemuxUnderDelay()
{
    // Breadth: MUX (2:1) and DEMUX (1:2) are select-driven combinational elements. Verify
    // correct routing and propagation delay in temporal mode.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *in0 = new InputSwitch();
    auto *in1 = new InputSwitch();
    auto *muxSel = new InputSwitch();
    auto *mux = new Mux();
    auto *demuxIn = new InputSwitch();
    auto *demuxSel = new InputSwitch();
    auto *demux = new Demux();
    builder.add(in0, in1, muxSel, mux);
    builder.add(demuxIn, demuxSel, demux);
    builder.connect(in0, 0, mux, 0);    // In0
    builder.connect(in1, 0, mux, 1);    // In1
    builder.connect(muxSel, 0, mux, 2); // S0
    builder.connect(demuxIn, 0, demux, 0);  // In
    builder.connect(demuxSel, 0, demux, 1); // S0
    Simulation *sim = builder.initSimulation();

    sim->setTimePerTick(100);
    const SimTime delay = 6;
    in0->setOn(false); // Out = S0 ? In1(1) : In0(0)  ⇒ Out tracks S0
    in1->setOn(true);
    muxSel->setOn(false);
    demuxIn->setOn(true);
    demuxSel->setOn(false);
    sim->update();
    sim->setElementDelay(mux, delay);
    sim->setElementDelay(demux, delay);

    // MUX: S0=0 ⇒ Out=In0=0; S0=1 ⇒ Out=In1=1.
    muxSel->setOn(false);
    sim->update();
    QCOMPARE(mux->outputValue(0), Status::Inactive);
    muxSel->setOn(true);
    sim->update();
    QCOMPARE(mux->outputValue(0), Status::Active);

    // DEMUX: In=1; S0=0 ⇒ Out0=1,Out1=0; S0=1 ⇒ Out0=0,Out1=1.
    demuxSel->setOn(false);
    sim->update();
    QCOMPARE(demux->outputValue(0), Status::Active);
    QCOMPARE(demux->outputValue(1), Status::Inactive);
    demuxSel->setOn(true);
    sim->update();
    QCOMPARE(demux->outputValue(0), Status::Inactive);
    QCOMPARE(demux->outputValue(1), Status::Active);

    // Confirm the MUX delay applies: flip S0 back to 0 with fine ticks; Out must not switch early.
    sim->setTimePerTick(1);
    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    const SimTime tEdge = sim->currentTime();
    muxSel->setOn(false); // Out should fall 1→0, after the delay
    advanceTo(tEdge + 2);
    QVERIFY2(mux->outputValue(0) == Status::Active, "MUX switched before its delay");
    advanceTo(tEdge + delay + 3);
    QCOMPARE(mux->outputValue(0), Status::Inactive);
}

void TestUnifiedTimed::testRestartDuringTemporalRun()
{
    // Lifecycle: restarting mid-temporal-run must reset sim time, rebuild/re-seed the whole
    // network, and leave temporal scheduling intact — timePerTick and the element's propagation
    // delay survive (restart keeps m_timePerTick; initialize() re-seeds m_delays from
    // propagationDelay()). The second run must therefore be timed identically to the first.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    notGate->setPropagationDelay(7); // element-level override (persists across restart)
    Simulation *sim = builder.initSimulation();
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    const auto runAndCheckTiming = [&]() {
        // sw=0 ⇒ NOT(0)=1; settle the (re-)seeded baseline.
        sw->setOn(false);
        advanceTo(sim->currentTime() + 20);
        QCOMPARE(notGate->outputValue(0), Status::Active);

        // sw→1 ⇒ NOT must fall to 0 only after its 7-unit delay.
        const SimTime tEdge = sim->currentTime();
        sw->setOn(true);
        advanceTo(tEdge + 3);
        QVERIFY2(notGate->outputValue(0) == Status::Active, "NOT flipped before its delay");
        advanceTo(tEdge + 7 + 3);
        QCOMPARE(notGate->outputValue(0), Status::Inactive);
    };

    // First temporal run.
    runAndCheckTiming();
    QVERIFY2(sim->currentTime() > 0, "Temporal time should have advanced");

    // Restart in the middle of the run.
    sim->restart();
    QCOMPARE(sim->currentTime(), SimTime(0)); // time reset

    // Second run WITHOUT re-applying timePerTick or the delay: both must have survived, and the
    // network must re-seed correctly, giving identical timed behaviour.
    runAndCheckTiming();
}

void TestUnifiedTimed::testXnorUnderDelay()
{
    // The last combinational gate not yet exercised under delay. XNOR(A,B) = (A == B).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *a = new InputSwitch();
    auto *b = new InputSwitch();
    auto *xnorGate = new Xnor();
    builder.add(a, b, xnorGate);
    builder.connect(a, 0, xnorGate, 0);
    builder.connect(b, 0, xnorGate, 1);
    Simulation *sim = builder.initSimulation();

    sim->setTimePerTick(100);
    const SimTime delay = 7;
    a->setOn(false);
    b->setOn(false);
    sim->update();
    sim->setElementDelay(xnorGate, delay);

    const auto settle = [&](bool av, bool bv) {
        a->setOn(av);
        b->setOn(bv);
        sim->update();
    };

    settle(true, true);   // equal ⇒ 1
    QCOMPARE(xnorGate->outputValue(0), Status::Active);
    settle(true, false);  // differ ⇒ 0
    QCOMPARE(xnorGate->outputValue(0), Status::Inactive);
    settle(false, false); // equal ⇒ 1
    QCOMPARE(xnorGate->outputValue(0), Status::Active);

    // Delay applies: with fine ticks, making the inputs differ must not flip the output early.
    sim->setTimePerTick(1);
    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    const SimTime tEdge = sim->currentTime();
    a->setOn(true); // now (1,0) ⇒ XNOR should fall 1→0, after the delay
    advanceTo(tEdge + 3);
    QVERIFY2(xnorGate->outputValue(0) == Status::Active, "XNOR flipped before its delay");
    advanceTo(tEdge + delay + 3);
    QCOMPARE(xnorGate->outputValue(0), Status::Inactive);
}

void TestUnifiedTimed::testNoSetupHoldViolation()
{
    // Deliberate omission, characterized: the model has no setup/hold window. Changing Data in the
    // SAME tick as the rising clock edge is not a violation — the flip-flop's delayed-capture
    // (m_simLastValue holds the pre-edge Data) yields a DEFINITE result, never metastable/Unknown.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *data = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ff = new DFlipFlop();
    builder.add(data, clk, ff);
    builder.connect(data, 0, ff, 0);
    builder.connect(clk, 0, ff, 1);
    Simulation *sim = builder.initSimulation(); // functional mode (timePerTick == 0)

    // Baseline: Data=0, clock low ⇒ Q=0, and the flip-flop has captured Data=0 as its pre-edge value.
    data->setOn(false);
    clk->setOn(false);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    // Simultaneous edge: raise Data AND the clock in the SAME update().
    data->setOn(true);
    clk->setOn(true);
    sim->update();

    // No metastability: Q is a definite value (not Unknown), and equals the pre-edge Data (0) —
    // the deterministic delayed-capture semantics, independent of within-tick evaluation order.
    QVERIFY2(ff->outputValue(0) != Status::Unknown, "Flip-flop went metastable — model should have no setup/hold");
    QCOMPARE(ff->outputValue(0), Status::Inactive);

    // The next clean edge (Data stable = 1) captures normally, confirming the FF is otherwise live.
    clk->setOn(false);
    sim->update();
    clk->setOn(true);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Active);
}

void TestUnifiedTimed::testDelayEqualToTickBoundary()
{
    // The drain processes events with time <= currentTime + timePerTick. So an event landing
    // EXACTLY on the tick boundary (delay == timePerTick) fires within that tick — the gate
    // settles in a single update; a delay one unit larger spills into the next tick.

    // Case A: delay == tick ⇒ settles in ONE update.
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
        QCOMPARE(notGate->outputValue(0), Status::Active); // ~0 = 1

        sim->setElementDelay(notGate, 5);
        sim->setTimePerTick(5);
        sw->setOn(true); // event scheduled at t+5 == this tick's targetTime
        sim->update();
        QCOMPARE(notGate->outputValue(0), Status::Inactive); // fired exactly on the boundary
    }

    // Case B: delay one unit past the tick ⇒ needs TWO updates.
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

        sim->setElementDelay(notGate, 6);
        sim->setTimePerTick(5);
        sw->setOn(true); // event at t+6 > this tick's targetTime (t+5)
        sim->update();
        QVERIFY2(notGate->outputValue(0) == Status::Active, "Event past the tick boundary fired too early");
        sim->update(); // next tick's window reaches t+6
        QCOMPARE(notGate->outputValue(0), Status::Inactive);
    }
}

void TestUnifiedTimed::testMixedDelayFeedbackLoopOscillates()
{
    // A ring mixing zero- and non-zero-delay gates: the zero-delay gate settles within a
    // timestamp, while the non-zero gates spread events across future timestamps and pace the
    // oscillation. So the loop oscillates over time (never the per-timestamp cap) — the middle
    // ground between the all-delayed ring (testRingOscillatorTemporal) and the all-zero-delay
    // ring (testZeroDelayLoopOscillatesToUnknownInTemporalMode, which caps to Unknown).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *n1 = new Not();
    auto *n2 = new Not();
    auto *n3 = new Not();
    builder.add(n1, n2, n3);
    builder.connect(n1, 0, n2, 0);
    builder.connect(n2, 0, n3, 0);
    builder.connect(n3, 0, n1, 0);
    Simulation *sim = builder.initSimulation();

    sim->setTimePerTick(1);
    sim->setElementDelay(n1, 6);
    sim->setElementDelay(n2, 3);
    sim->setElementDelay(n3, 0); // mixed: one zero-delay gate in the loop

    int transitions = 0;
    bool sawUnknown = false;
    Status prev = n1->outputValue(0);
    for (int i = 0; i < 150; ++i) {
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

    QVERIFY2(!sawUnknown, "Mixed-delay loop hit the oscillation cap and went Unknown");
    QVERIFY2(transitions >= 3, qPrintable(
        QString("Mixed-delay loop did not oscillate: only %1 transitions").arg(transitions)));
}

void TestUnifiedTimed::testAsyncClearDominatesSimultaneousClockEdge()
{
    // When ~Clear is asserted in the SAME tick as a rising clock edge, the asynchronous clear
    // wins: DFlipFlop::updateLogic applies the clock capture, then the preset/clear override runs
    // afterwards and forces Q low. Functional mode (one settle per update).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *data = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *nClear = new InputSwitch();
    auto *ff = new DFlipFlop();
    builder.add(data, clk, nClear, ff);
    builder.connect(data, 0, ff, 0);   // Data
    builder.connect(clk, 0, ff, 1);    // Clock
    builder.connect(nClear, 0, ff, 3); // ~Clear (active low)
    Simulation *sim = builder.initSimulation();

    // Clock a 1 into Q with ~Clear deasserted.
    nClear->setOn(true);
    data->setOn(true);
    clk->setOn(false);
    sim->update();
    clk->setOn(true);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Active); // Q = 1

    // Coincident rising edge + ~Clear assertion in ONE update.
    clk->setOn(false);
    sim->update();
    clk->setOn(true);   // rising edge that would capture Data=1...
    nClear->setOn(false); // ...but ~Clear is asserted in the same tick
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive); // clear dominates the edge
    QCOMPARE(ff->outputValue(1), Status::Active);   // ~Q

    // With ~Clear released, a fresh edge captures Data=1 again — the FF is otherwise live.
    nClear->setOn(true);
    clk->setOn(false);
    sim->update();
    clk->setOn(true);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Active);
}

void TestUnifiedTimed::testLargeDelayHonoredAtFullWidth()
{
    // SimTime is uint64_t. A delay above 2^32 must fire at its true time, not a 32-bit-truncated
    // one. The drain is event-based, so a multi-billion-ns delay costs only a few updates here.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    builder.add(sw, notGate);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    sw->setOn(false);
    sim->update();
    QCOMPARE(notGate->outputValue(0), Status::Active); // ~0 = 1

    const SimTime bigDelay = 5'000'000'000ULL;   // > 2^32 (4'294'967'296)
    const SimTime tick = 1'000'000'000ULL;
    sim->setElementDelay(notGate, bigDelay);
    sim->setTimePerTick(tick);
    sw->setOn(true); // event scheduled at t + 5e9

    // After one tick (currentTime 1e9) the event has NOT fired. A 32-bit-truncated delay
    // (≈ 705 M) would already have flipped the NOT here — proving the full width is honored.
    sim->update();
    QCOMPARE(sim->currentTime(), tick);
    QVERIFY2(notGate->outputValue(0) == Status::Active, "Large delay fired too early (truncated?)");

    // Reach 5e9: the event fires on the fifth tick.
    for (int i = 0; i < 4; ++i) {
        sim->update();
    }
    QCOMPARE(sim->currentTime(), bigDelay); // no overflow/truncation
    QCOMPARE(notGate->outputValue(0), Status::Inactive); // ~1 = 0, exactly at t + 5e9
}

void TestUnifiedTimed::testFlipFlopComplementaryOutputsChangeTogether()
{
    // A flip-flop sets Q and ~Q in one updateLogic, so under a propagation delay both outputs
    // must flip at the SAME delayed timestamp — the per-element delay applies to every port.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *data = new InputSwitch();
    auto *clk = new InputSwitch();
    auto *ff = new DFlipFlop();
    builder.add(data, clk, ff);
    builder.connect(data, 0, ff, 0);
    builder.connect(clk, 0, ff, 1);
    Simulation *sim = builder.initSimulation();

    // Baseline: Q=0, ~Q=1.
    data->setOn(true);
    clk->setOn(false);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);
    QCOMPARE(ff->outputValue(1), Status::Active);

    const SimTime delay = 10;
    sim->setElementDelay(ff, delay);
    sim->setTimePerTick(1);

    const auto advanceTo = [sim](SimTime target) {
        while (sim->currentTime() < target) {
            sim->update();
        }
    };
    advanceTo(sim->currentTime() + 2 * delay); // settle into temporal mode, capture Data=1

    const SimTime tEdge = sim->currentTime();
    clk->setOn(true); // rising edge ⇒ Q→1, ~Q→0 after the delay

    // Before the delay: BOTH outputs unchanged.
    advanceTo(tEdge + (delay - 2));
    QCOMPARE(ff->outputValue(0), Status::Inactive);
    QCOMPARE(ff->outputValue(1), Status::Active);

    // After the delay: BOTH outputs flipped, together.
    advanceTo(tEdge + delay + 3);
    QCOMPARE(ff->outputValue(0), Status::Active);
    QCOMPARE(ff->outputValue(1), Status::Inactive);
}

void TestUnifiedTimed::testSimultaneousInputArrivalNoGlitch()
{
    // Two inputs changing in the SAME tick arrive at the gate simultaneously (equal direct
    // paths), so the gate evaluates with both already settled — no transient. This is the
    // complement to testReconvergentFanoutHazard, where unequal path delays DO glitch.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *a = new InputSwitch();
    auto *b = new InputSwitch();
    auto *xorGate = new Xor();
    builder.add(a, b, xorGate);
    builder.connect(a, 0, xorGate, 0);
    builder.connect(b, 0, xorGate, 1);
    Simulation *sim = builder.initSimulation();

    sim->setTimePerTick(100);
    sim->setElementDelay(xorGate, 5);
    a->setOn(true);
    b->setOn(false);
    sim->update(); // A^B = 1
    QCOMPARE(xorGate->outputValue(0), Status::Active);

    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("XOR", xorGate, 0);
    recorder.setRecording(true);

    // Flip BOTH inputs in one update: A→0 and B→1 simultaneously. 0^1 == 1, so XOR is unchanged —
    // but only if both inputs are seen settled. A skewed evaluation would briefly read (0,0)=0.
    a->setOn(false);
    b->setOn(true);
    sim->update();

    const auto &transitions = recorder.trace(0).transitions;
    for (const auto &tr : transitions) {
        QVERIFY2(tr.second != Status::Inactive,
                 "XOR glitched to 0 — simultaneous input arrival should be transient-free");
    }
    QCOMPARE(xorGate->outputValue(0), Status::Active);
}
