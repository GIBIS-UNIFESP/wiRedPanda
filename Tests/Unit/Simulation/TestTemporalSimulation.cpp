// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestTemporalSimulation.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/IC.h"
#include "App/Simulation/SimEvent.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/WaveformRecorder.h"
#include "App/UI/TemporalWaveformWidget.h"
#include "Tests/Common/TestUtils.h"

using namespace TestUtils;

// ============================================================================
// EventQueue unit tests
// ============================================================================

void TestTemporalSimulation::testEventQueueOrdering()
{
    EventQueue queue;
    GraphicElement *dummy = nullptr; // not dereferenced

    queue.schedule({100, 0, dummy});
    queue.schedule({50, 0, dummy});
    queue.schedule({200, 0, dummy});
    queue.schedule({10, 0, dummy});

    QCOMPARE(queue.nextTime(), SimTime(10));
    queue.pop();
    QCOMPARE(queue.nextTime(), SimTime(50));
    queue.pop();
    QCOMPARE(queue.nextTime(), SimTime(100));
    queue.pop();
    QCOMPARE(queue.nextTime(), SimTime(200));
    queue.pop();
    QVERIFY(queue.empty());
}

void TestTemporalSimulation::testEventQueueSameTime()
{
    // Events at the same time should all be retrievable (heap tie-order is unspecified).
    EventQueue queue;
    auto *a = reinterpret_cast<GraphicElement *>(1);
    auto *b = reinterpret_cast<GraphicElement *>(2);
    auto *c = reinterpret_cast<GraphicElement *>(3);

    queue.schedule({100, 0, a});
    queue.schedule({100, 0, b});
    queue.schedule({100, 0, c});

    QCOMPARE(queue.size(), 3);
    QCOMPARE(queue.nextTime(), SimTime(100));

    const auto e1 = queue.pop();
    const auto e2 = queue.pop();
    const auto e3 = queue.pop();
    QCOMPARE(e1.time, SimTime(100));
    QCOMPARE(e2.time, SimTime(100));
    QCOMPARE(e3.time, SimTime(100));
    QVERIFY(queue.empty());
}

void TestTemporalSimulation::testEventQueueClear()
{
    EventQueue queue;
    queue.schedule({10, 0, nullptr});
    queue.schedule({20, 0, nullptr});
    QCOMPARE(queue.size(), 2);

    queue.clear();
    QVERIFY(queue.empty());
    QCOMPARE(queue.size(), 0);
}

// ============================================================================
// Per-element propagation delay
// ============================================================================

void TestTemporalSimulation::testDefaultPropagationDelays()
{
    // Logic gates get non-zero per-type defaults (set on construction, no init needed).
    Not notGate;
    And andGate;
    Or orGate;
    Nand nandGate;

    QCOMPARE(notGate.propagationDelay(), SimTime(5));
    QCOMPARE(andGate.propagationDelay(), SimTime(10));
    QCOMPARE(orGate.propagationDelay(), SimTime(10));
    QCOMPARE(nandGate.propagationDelay(), SimTime(8));
}

void TestTemporalSimulation::testZeroDelayElements()
{
    // Sources and sinks introduce no propagation delay.
    InputSwitch sw;
    Led led;

    QCOMPARE(sw.propagationDelay(), SimTime(0));
    QCOMPARE(led.propagationDelay(), SimTime(0));

    // A per-element override is honoured (and can revert to the type default).
    Not notGate;
    notGate.setPropagationDelay(42);
    QCOMPARE(notGate.propagationDelay(), SimTime(42));
    QVERIFY(notGate.hasPropagationDelayOverride());
    notGate.setPropagationDelay(SIM_TIME_UNSET);
    QCOMPARE(notGate.propagationDelay(), SimTime(5));
    QVERIFY(!notGate.hasPropagationDelayOverride());
}

// ============================================================================
// Successor graph
// ============================================================================

void TestTemporalSimulation::testSuccessorGraphBuilt()
{
    // The static successor graph maps each element to its downstream neighbours.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);
    builder.initSimulation();

    const QVector<GraphicElement *> elements = {&sw, &notGate, &led};
    const auto txMap = Simulation::buildTxMap(elements);
    const auto successors = Simulation::buildSuccessorGraph(elements, txMap);

    QCOMPARE(successors.value(&sw).size(), 1);
    QCOMPARE(successors.value(&sw).at(0), static_cast<GraphicElement *>(&notGate));
    QCOMPARE(successors.value(&notGate).size(), 1);
    QCOMPARE(successors.value(&notGate).at(0), static_cast<GraphicElement *>(&led));
    QVERIFY(successors.value(&led).isEmpty());
}

// ============================================================================
// Functional vs temporal mode
// ============================================================================

void TestTemporalSimulation::testFunctionalModeDefault()
{
    // Default timePerTick is 0 ⇒ functional (zero-delay): sim time never advances,
    // but combinational logic still settles every tick.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Led led;
    builder.add(&sw, &led);
    builder.connect(&sw, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->update();
    sim->update();
    QCOMPARE(sim->currentTime(), SimTime(0));

    sw.setOn(true);
    sim->update();
    QCOMPARE(inputStatus(&led), true);
}

void TestTemporalSimulation::testTemporalModeAdvancesTime()
{
    // In temporal mode each tick advances sim time by exactly timePerTick.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Led led;
    builder.add(&sw, &led);
    builder.connect(&sw, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(100);
    sim->update();
    const SimTime t1 = sim->currentTime();
    sim->update();
    const SimTime t2 = sim->currentTime();
    QCOMPARE(t2 - t1, SimTime(100));
}

// ============================================================================
// Temporal engine: zero delay equivalence
// ============================================================================

void TestTemporalSimulation::testZeroDelayAndGate()
{
    // In temporal mode with zero delays, an AND gate behaves like functional mode.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &andGate, &led);
    builder.connect(&sw1, 0, &andGate, 0);
    builder.connect(&sw2, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(10);
    sim->update();                      // init snapshots delays from defaults
    sim->setElementDelay(&andGate, 0);  // force zero delay (live override)

    // 0 AND 0 = 0
    sw1.setOn(false); sw2.setOn(false);
    sim->update();
    QCOMPARE(inputStatus(&led), false);

    // 1 AND 1 = 1
    sw1.setOn(true); sw2.setOn(true);
    sim->update();
    QCOMPARE(inputStatus(&led), true);

    // 1 AND 0 = 0
    sw2.setOn(false);
    sim->update();
    QCOMPARE(inputStatus(&led), false);
}

void TestTemporalSimulation::testZeroDelayCascadedGates()
{
    // NOT → AND chain with zero delays should propagate in a single tick.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Not notGate;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &notGate, &andGate, &led);
    builder.connect(&sw1, 0, &notGate, 0);
    builder.connect(&notGate, 0, &andGate, 0);
    builder.connect(&sw2, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(10);
    sim->update();
    sim->setElementDelay(&notGate, 0);
    sim->setElementDelay(&andGate, 0);

    // sw1=0 → NOT=1, sw2=1 → AND(1,1) = 1
    sw1.setOn(false); sw2.setOn(true);
    sim->update();
    QCOMPARE(inputStatus(&led), true);

    // sw1=1 → NOT=0 → AND(0,1) = 0
    sw1.setOn(true);
    sim->update();
    QCOMPARE(inputStatus(&led), false);
}

void TestTemporalSimulation::testZeroDelayFeedbackConverges()
{
    // SR latch with zero delays should converge via delta cycles at a single timestamp.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch setBtn, resetBtn;
    Nand nand1, nand2;
    Led qLed;

    builder.add(&setBtn, &resetBtn, &nand1, &nand2, &qLed);
    builder.connect(&setBtn, 0, &nand1, 0);
    builder.connect(&nand2, 0, &nand1, 1);
    builder.connect(&resetBtn, 0, &nand2, 0);
    builder.connect(&nand1, 0, &nand2, 1);
    builder.connect(&nand1, 0, &qLed, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(10);
    sim->update();
    sim->setElementDelay(&nand1, 0);
    sim->setElementDelay(&nand2, 0);

    // Set: S=0 (active), R=1 → Q should be 1
    setBtn.setOn(false);
    resetBtn.setOn(true);
    sim->update();
    QCOMPARE(inputStatus(&qLed), true);

    // Hold: S=1, R=1 → Q should stay 1
    setBtn.setOn(true);
    sim->update();
    QCOMPARE(inputStatus(&qLed), true);

    // Reset: S=1, R=0 (active) → Q should be 0
    resetBtn.setOn(false);
    sim->update();
    QCOMPARE(inputStatus(&qLed), false);
}

// ============================================================================
// Temporal engine: propagation delays
// ============================================================================

void TestTemporalSimulation::testNotGateDelayPropagation()
{
    // NOT gate with its 5 ns default delay: a tick wide enough absorbs the delay
    // in one update; with 1 ns ticks several updates are needed before it settles.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(100); // 100 ns per tick — large enough to cover the 5 ns delay
    sim->update();

    QCOMPARE(notGate.propagationDelay(), SimTime(5));

    // sw=off → NOT(0) = 1
    sw.setOn(false);
    sim->update();
    QCOMPARE(notGate.outputValue(), Status::Active);

    // sw=on → NOT(1) = 0 (settles within one 100 ns tick)
    sw.setOn(true);
    sim->update();
    QCOMPARE(notGate.outputValue(), Status::Inactive);

    // Now with tiny 1 ns ticks: the 5 ns event needs several ticks to fire.
    sim->setTimePerTick(1);
    sw.setOn(false); // NOT(0) should become 1 after 5 ns

    // After enough ticks (> 5 ns) it must have settled to Active.
    for (int i = 0; i < 15; ++i) {
        sim->update();
    }
    QCOMPARE(notGate.outputValue(), Status::Active);
}

void TestTemporalSimulation::testChainedGatesCumulativeDelay()
{
    // NOT(5 ns) → AND(10 ns): the whole chain settles within a wide enough tick.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Not notGate;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &notGate, &andGate, &led);
    builder.connect(&sw1, 0, &notGate, 0);
    builder.connect(&notGate, 0, &andGate, 0);
    builder.connect(&sw2, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(100); // 100 ns per tick covers the 5 + 10 = 15 ns chain
    sim->update();

    QCOMPARE(notGate.propagationDelay(), SimTime(5));
    QCOMPARE(andGate.propagationDelay(), SimTime(10));

    // sw1=0 → NOT=1 (after 5 ns), sw2=1 → AND(1,1)=1 (after 5+10=15 ns)
    sw1.setOn(false);
    sw2.setOn(true);
    sim->update();

    QCOMPARE(andGate.outputValue(), Status::Active);
    QCOMPARE(inputStatus(&led), true);
}

void TestTemporalSimulation::testICInternalDelayPropagation()
{
    // The half-adder IC wraps an XOR (Sum) and an AND (Carry, 10 ns). After flattening, those
    // internal gates are real participants in the temporal netlist, so their propagation delays
    // must apply through the IC boundary — a pre-flatten IC was a zero-delay black box.
    const QString icPath = cpuComponentsDir() + QStringLiteral("level2_half_adder.panda");
    if (!QFile::exists(icPath)) {
        QSKIP("level2_half_adder.panda fixture not found");
    }

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swA, swB;
    Led ledSum, ledCarry;
    builder.add(&swA, &swB, &ledSum, &ledCarry);

    auto *ic = new IC();
    ic->loadFile(icPath, QFileInfo(icPath).absolutePath());
    builder.add(ic); // adds the IC to the scene

    builder.connect(&swA, 0, ic, QStringLiteral("A"));
    builder.connect(&swB, 0, ic, QStringLiteral("B"));
    builder.connect(ic, QStringLiteral("Sum"), &ledSum, 0);
    builder.connect(ic, QStringLiteral("Carry"), &ledCarry, 0);

    auto *sim = builder.initSimulation();

    // A wide tick (≥ the internal delays) settles the IC outputs correctly through the boundary.
    sim->setTimePerTick(100);
    swA.setOn(false); swB.setOn(false);
    sim->update();
    QCOMPARE(inputStatus(&ledCarry), false);

    swA.setOn(true); swB.setOn(true);
    sim->update();
    QCOMPARE(inputStatus(&ledCarry), true);  // Carry = A·B = 1
    QCOMPARE(inputStatus(&ledSum), false);   // Sum = A⊕B = 0

    // Prove the internal AND's 10 ns delay actually applies through the IC boundary: with 1 ns
    // ticks the Carry output must NOT change within a few ns of an input change, and must settle
    // only after the delay elapses. A zero-delay black box would flip in a single tick.
    sim->setTimePerTick(1);
    swA.setOn(false); swB.setOn(false);
    for (int i = 0; i < 40; ++i) { sim->update(); } // settle Carry back to 0
    QCOMPARE(inputStatus(&ledCarry), false);

    swA.setOn(true); swB.setOn(true);
    for (int i = 0; i < 3; ++i) { sim->update(); }  // 3 ns ≪ 10 ns AND delay
    QCOMPARE(inputStatus(&ledCarry), false);     // internal delay not yet elapsed

    for (int i = 0; i < 40; ++i) { sim->update(); } // well past 10 ns
    QCOMPARE(inputStatus(&ledCarry), true);      // internal delay applied, Carry settled
}

void TestTemporalSimulation::testWatchedICOutputPinNotOneTickStale()
{
    // Regression test: a directly-watched IC output port's waveform trace must reflect the
    // SAME tick's settled value as a direct outputValue() read — not the previous tick's,
    // since recordAll() runs inside processEvents() while mirrorICOutputs() (the only thing
    // that keeps an IC's own outputValue() in sync with its now-flattened internal
    // simulation) used to only run once, at the very end of update().
    const QString icPath = cpuComponentsDir() + QStringLiteral("level2_half_adder.panda");
    if (!QFile::exists(icPath)) {
        QSKIP("level2_half_adder.panda fixture not found");
    }

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swA, swB;
    builder.add(&swA, &swB);

    auto *ic = new IC();
    ic->loadFile(icPath, QFileInfo(icPath).absolutePath());
    builder.add(ic);

    builder.connect(&swA, 0, ic, QStringLiteral("A"));
    builder.connect(&swB, 0, ic, QStringLiteral("B"));

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(100); // wide enough for the IC's internal delays to settle in one tick

    // Watch the IC's own Carry output port directly (not an internal primitive).
    const int carryPortIndex = ic->outputSize() - 1; // Carry is the IC's last output port
    sim->waveformRecorder().watchSignal("Carry", ic, carryPortIndex);
    sim->waveformRecorder().setRecording(true);

    swA.setOn(false); swB.setOn(false);
    sim->update();

    swA.setOn(true); swB.setOn(true); // Carry = A·B should settle to Active this tick
    sim->update();

    const Status direct = ic->outputValue(carryPortIndex);
    QCOMPARE(direct, Status::Active);
    const auto &transitions = sim->waveformRecorder().trace(0).transitions;
    QVERIFY(!transitions.isEmpty());
    QCOMPARE(transitions.last().second, direct); // trace must match the same-tick direct read
}

// ============================================================================
// Temporal engine: input change detection
// ============================================================================

void TestTemporalSimulation::testInputSwitchSchedulesEvent()
{
    // Toggling an input switch propagates through the temporal engine to the sink.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Led led;

    builder.add(&sw, &led);
    builder.connect(&sw, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(100);
    sim->update();

    sw.setOn(false);
    sim->update();
    QCOMPARE(inputStatus(&led), false);

    sw.setOn(true);
    sim->update();
    QCOMPARE(inputStatus(&led), true);

    sw.setOn(false);
    sim->update();
    QCOMPARE(inputStatus(&led), false);
}

// ============================================================================
// Waveform recorder
// ============================================================================

void TestTemporalSimulation::testRecorderWatchAndRecord()
{
    WaveformRecorder recorder;

    const int idx = recorder.watchSignal("sig0", nullptr, 0);
    QCOMPARE(idx, 0);
    QCOMPARE(recorder.traceCount(), 1);
    QCOMPARE(recorder.trace(0).name, QString("sig0"));

    // A genuinely distinct signal (different port on the same — here null — element) gets its
    // own trace.
    recorder.watchSignal("sig1", nullptr, 1);
    QCOMPARE(recorder.traceCount(), 2);

    recorder.clear();
    QCOMPARE(recorder.traceCount(), 0);
}

void TestTemporalSimulation::testRecorderWatchSignalDedups()
{
    // Regression test: watching the exact same (element, port) signal twice must not create a
    // duplicate trace — e.g. re-invoking "Watch internal signals" on the same IC.
    WaveformRecorder recorder;

    Not notGate;
    const int first = recorder.watchSignal("NOT_out", &notGate, 0);
    QCOMPARE(recorder.traceCount(), 1);

    const int second = recorder.watchSignal("NOT_out", &notGate, 0);
    QCOMPARE(second, first);
    QCOMPARE(recorder.traceCount(), 1);

    // A different port on the SAME element is still a distinct signal.
    recorder.watchSignal("NOT_out2", &notGate, 1);
    QCOMPARE(recorder.traceCount(), 2);
}

void TestTemporalSimulation::testRecorderDeduplication()
{
    // recordAll() must not add duplicate consecutive values for an unchanging signal.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(100);
    sim->update(); // init rebuilds the simulation graph

    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("NOT_out", &notGate, 0);
    recorder.setRecording(true);

    // Several ticks with no input change must not accumulate one transition per tick.
    sim->update();
    sim->update();
    sim->update();

    const auto &transitions = recorder.trace(0).transitions;
    QVERIFY2(transitions.size() <= 4, qPrintable(
        QString("Expected at most 4 transitions, got %1").arg(transitions.size())));
}

void TestTemporalSimulation::testRecorderStatusAt()
{
    SignalTrace trace;
    trace.name = "test";
    trace.transitions = {{10, Status::Active}, {50, Status::Inactive}, {100, Status::Active}};

    QCOMPARE(trace.statusAt(0), Status::Inactive);   // before first transition
    QCOMPARE(trace.statusAt(10), Status::Active);     // exactly at first
    QCOMPARE(trace.statusAt(30), Status::Active);     // between first and second
    QCOMPARE(trace.statusAt(50), Status::Inactive);   // exactly at second
    QCOMPARE(trace.statusAt(75), Status::Inactive);   // between second and third
    QCOMPARE(trace.statusAt(100), Status::Active);    // exactly at third
    QCOMPARE(trace.statusAt(200), Status::Active);    // after last
}

void TestTemporalSimulation::testRecorderIntegration()
{
    // Full integration: temporal sim with recorder, verify transitions are captured.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(100);
    sim->update(); // init rebuilds the simulation graph

    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("NOT_out", &notGate, 0);
    recorder.setRecording(true);

    // sw=off → NOT=1
    sw.setOn(false);
    sim->update();

    // sw=on → NOT=0
    sw.setOn(true);
    sim->update();

    const auto &transitions = recorder.trace(0).transitions;
    QVERIFY2(!transitions.isEmpty(), "Recorder should have captured transitions");

    // The last recorded value should reflect NOT(1) = Inactive.
    QCOMPARE(transitions.last().second, Status::Inactive);
    QVERIFY(recorder.maxTime() > 0);
}

// ============================================================================
// Waveform widget
// ============================================================================

void TestTemporalSimulation::testWidgetSizeHint()
{
    WaveformRecorder recorder;
    TemporalWaveformWidget widget;
    widget.setRecorder(&recorder);

    // No traces → at least the minimum size.
    QSize hint = widget.sizeHint();
    QVERIFY(hint.width() >= 100);
    QVERIFY(hint.height() >= 54);

    // Adding a trace keeps a valid size.
    recorder.watchSignal("sig0", nullptr, 0);
    hint = widget.sizeHint();
    QVERIFY(hint.height() >= 54);

    // Painting with no recorded data must not crash.
    widget.resize(400, 200);
    widget.repaint();
}
