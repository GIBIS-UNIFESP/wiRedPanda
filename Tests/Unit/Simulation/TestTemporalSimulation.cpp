// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestTemporalSimulation.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
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
    QCOMPARE(getInputStatus(&led), true);
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
    QCOMPARE(getInputStatus(&led), false);

    // 1 AND 1 = 1
    sw1.setOn(true); sw2.setOn(true);
    sim->update();
    QCOMPARE(getInputStatus(&led), true);

    // 1 AND 0 = 0
    sw2.setOn(false);
    sim->update();
    QCOMPARE(getInputStatus(&led), false);
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
    QCOMPARE(getInputStatus(&led), true);

    // sw1=1 → NOT=0 → AND(0,1) = 0
    sw1.setOn(true);
    sim->update();
    QCOMPARE(getInputStatus(&led), false);
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
    QCOMPARE(getInputStatus(&qLed), true);

    // Hold: S=1, R=1 → Q should stay 1
    setBtn.setOn(true);
    sim->update();
    QCOMPARE(getInputStatus(&qLed), true);

    // Reset: S=1, R=0 (active) → Q should be 0
    resetBtn.setOn(false);
    sim->update();
    QCOMPARE(getInputStatus(&qLed), false);
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
    QCOMPARE(getInputStatus(&led), true);
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
    QCOMPARE(getInputStatus(&led), false);

    sw.setOn(true);
    sim->update();
    QCOMPARE(getInputStatus(&led), true);

    sw.setOn(false);
    sim->update();
    QCOMPARE(getInputStatus(&led), false);
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

    recorder.watchSignal("sig1", nullptr, 0);
    QCOMPARE(recorder.traceCount(), 2);

    recorder.clear();
    QCOMPARE(recorder.traceCount(), 0);
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
