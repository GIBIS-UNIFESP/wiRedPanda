// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestUnifiedTimed.h"

#include <QtTest>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Not.h"
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
