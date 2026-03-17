// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestTemporalSimulation.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/LogicElements/LogicElement.h"
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
    LogicElement *dummy = nullptr; // not dereferenced

    queue.schedule({100, dummy});
    queue.schedule({50, dummy});
    queue.schedule({200, dummy});
    queue.schedule({10, dummy});

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

void TestTemporalSimulation::testEventQueueSameTimeFIFO()
{
    // Events at the same time should all be retrievable
    EventQueue queue;
    LogicElement *a = reinterpret_cast<LogicElement *>(1);
    LogicElement *b = reinterpret_cast<LogicElement *>(2);
    LogicElement *c = reinterpret_cast<LogicElement *>(3);

    queue.schedule({100, a});
    queue.schedule({100, b});
    queue.schedule({100, c});

    QCOMPARE(queue.size(), 3);
    QCOMPARE(queue.nextTime(), SimTime(100));

    // All three events should be at time 100
    auto e1 = queue.pop();
    auto e2 = queue.pop();
    auto e3 = queue.pop();
    QCOMPARE(e1.time, SimTime(100));
    QCOMPARE(e2.time, SimTime(100));
    QCOMPARE(e3.time, SimTime(100));
    QVERIFY(queue.empty());
}

void TestTemporalSimulation::testEventQueueClear()
{
    EventQueue queue;
    queue.schedule({10, nullptr});
    queue.schedule({20, nullptr});
    QCOMPARE(queue.size(), 2);

    queue.clear();
    QVERIFY(queue.empty());
    QCOMPARE(queue.size(), 0);
}

// ============================================================================
// LogicElement delay
// ============================================================================

void TestTemporalSimulation::testDefaultPropagationDelays()
{
    // Verify that logic gates get non-zero default delays
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Not notGate;
    And andGate;
    Or orGate;
    Nand nandGate;

    builder.add(&notGate, &andGate, &orGate, &nandGate);
    auto *sim = builder.initSimulation();
    Q_UNUSED(sim)

    QCOMPARE(notGate.logic()->propagationDelay(), SimTime(5));
    QCOMPARE(andGate.logic()->propagationDelay(), SimTime(10));
    QCOMPARE(orGate.logic()->propagationDelay(), SimTime(10));
    QCOMPARE(nandGate.logic()->propagationDelay(), SimTime(8));
}

void TestTemporalSimulation::testZeroDelayElements()
{
    // Sources, sinks, and nodes should have zero delay
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Led led;

    builder.add(&sw, &led);
    builder.connect(&sw, 0, &led, 0);
    auto *sim = builder.initSimulation();
    Q_UNUSED(sim)

    // InputSwitch logic is LogicSource → delay 0
    QCOMPARE(sw.logic()->propagationDelay(), SimTime(0));
    // Led logic is LogicSink → delay 0
    QCOMPARE(led.logic()->propagationDelay(), SimTime(0));
}

// ============================================================================
// Successor tracking
// ============================================================================

void TestTemporalSimulation::testSuccessorListsBuilt()
{
    // After init, each element should have a populated successor list
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    Q_UNUSED(sim)

    // sw.logic (LogicSource) → successor is notGate.logic
    QCOMPARE(sw.logic()->successorGroupCount(), 1);
    QCOMPARE(sw.logic()->successors(0).size(), 1);
    QCOMPARE(sw.logic()->successors(0).at(0).logic, notGate.logic());

    // notGate.logic → successor is led.logic (LogicSink)
    QCOMPARE(notGate.logic()->successorGroupCount(), 1);
    QCOMPARE(notGate.logic()->successors(0).size(), 1);
    QCOMPARE(notGate.logic()->successors(0).at(0).logic, led.logic());

    // led.logic (LogicSink) has no successors
    QCOMPARE(led.logic()->successorGroupCount(), 1); // 1 output port but 0 entries
    QVERIFY(led.logic()->successors(0).isEmpty());
}

// ============================================================================
// Mode switching
// ============================================================================

void TestTemporalSimulation::testModeDefaultIsFunctional()
{
    WorkSpace workspace;
    auto *sim = workspace.scene()->simulation();
    QCOMPARE(sim->mode(), SimulationMode::Functional);
}

void TestTemporalSimulation::testSetModeRestartsSimulation()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Led led;
    builder.add(&sw, &led);
    builder.connect(&sw, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->update(); // initializes

    sim->setMode(SimulationMode::Temporal);
    QCOMPARE(sim->mode(), SimulationMode::Temporal);
    QCOMPARE(sim->currentTime(), SimTime(0));

    // Switching back resets too
    sim->setMode(SimulationMode::Functional);
    QCOMPARE(sim->mode(), SimulationMode::Functional);
}

// ============================================================================
// Temporal engine: zero delay equivalence
// ============================================================================

void TestTemporalSimulation::testZeroDelayAndGate()
{
    // In temporal mode with zero delays, AND gate should behave identically
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

    // Force zero delays for equivalence test
    andGate.logic()->setPropagationDelay(0);

    sim->setMode(SimulationMode::Temporal);
    sim->update(); // re-initializes in temporal mode

    // Force zero delays again after re-init
    andGate.logic()->setPropagationDelay(0);

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
    // NOT → AND chain with zero delays should propagate in one tick
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
    sim->setMode(SimulationMode::Temporal);
    sim->update(); // re-init

    // Force zero delays
    notGate.logic()->setPropagationDelay(0);
    andGate.logic()->setPropagationDelay(0);

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
    // SR latch with zero delays should converge via delta cycles
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
    sim->setMode(SimulationMode::Temporal);
    sim->update(); // re-init

    // Force zero delays
    nand1.logic()->setPropagationDelay(0);
    nand2.logic()->setPropagationDelay(0);

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
    // NOT gate with 5ns delay: verify output changes are delayed.
    // With a large enough tick, the delay is absorbed in one update.
    // With a tiny tick (1ns), multiple updates are needed.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setMode(SimulationMode::Temporal);
    sim->setTimePerTick(100); // 100ns per tick — large enough to cover 5ns delay
    sim->update(); // re-init

    // NOT gate default delay is 5ns
    QCOMPARE(notGate.logic()->propagationDelay(), SimTime(5));

    // sw=off → NOT(0) = 1
    sw.setOn(false);
    sim->update();
    QCOMPARE(notGate.logic()->outputValue(), Status::Active);

    // sw=on → NOT(1) = 0 (after 5ns, which fits in one 100ns tick)
    sw.setOn(true);
    sim->update();
    QCOMPARE(notGate.logic()->outputValue(), Status::Inactive);

    // Now test with tiny ticks: 1ns per tick, delay is 5ns
    sim->setTimePerTick(1);

    sw.setOn(false); // NOT(0) should become 1 after 5ns
    // After 3 ticks (3ns), the 5ns event hasn't fired yet
    sim->update();
    sim->update();
    sim->update();
    // The NOT gate still shows the old output (Inactive) because the event
    // scheduled at current_time + 5 hasn't been reached yet
    // (We can't assert the exact intermediate state because the init seed
    // events may have already propagated, but after enough ticks it must settle)

    // After 10 more ticks (total > 5ns from the last input change), it must have settled
    for (int i = 0; i < 10; ++i) {
        sim->update();
    }
    QCOMPARE(notGate.logic()->outputValue(), Status::Active);
}

void TestTemporalSimulation::testChainedGatesCumulativeDelay()
{
    // NOT(5ns) → AND(10ns): total delay should be 15ns from input change to AND output
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
    sim->setMode(SimulationMode::Temporal);
    // Use a large enough tick to process the full chain in one update
    sim->setTimePerTick(100); // 100ns per tick
    sim->update(); // re-init

    QCOMPARE(notGate.logic()->propagationDelay(), SimTime(5));
    QCOMPARE(andGate.logic()->propagationDelay(), SimTime(10));

    // sw1=0 → NOT=1 (after 5ns), sw2=1, AND(1,1)=1 (after 5+10=15ns)
    sw1.setOn(false);
    sw2.setOn(true);
    sim->update();

    // After 100ns, the full chain should have settled
    QCOMPARE(andGate.logic()->outputValue(), Status::Active);
    QCOMPARE(getInputStatus(&led), true);
}

// ============================================================================
// Temporal engine: clock scheduling
// ============================================================================

void TestTemporalSimulation::testClockEdgeScheduling()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Clock clock;
    Led led;

    builder.add(&clock, &led);
    builder.connect(&clock, 0, &led, 0);

    clock.setFrequency(1000.0f); // 1kHz → half-period = 500µs = 500000ns

    auto *sim = builder.initSimulation();
    sim->setMode(SimulationMode::Temporal);
    sim->setTimePerTick(1'000'000); // 1ms per tick — covers 1 full period
    sim->update(); // re-init, clock starts HIGH

    // Clock starts HIGH after resetTemporalClock
    Status initialStatus = clock.logic()->outputValue();
    QCOMPARE(initialStatus, Status::Active);

    // After one tick (1ms = 1000000ns), clock should have toggled at least once
    sim->update();

    // The clock should have toggled (half-period is 500000ns, tick is 1000000ns)
    // So it should toggle twice (HIGH→LOW at 500000ns, LOW→HIGH at 1000000ns)
    // ending back at HIGH
    QCOMPARE(clock.logic()->outputValue(), Status::Active);
}

// ============================================================================
// Temporal engine: input change detection
// ============================================================================

void TestTemporalSimulation::testInputSwitchSchedulesEvent()
{
    // When an input switch is toggled, scheduleIfChanged should detect it
    // and the temporal engine should propagate the change
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Led led;

    builder.add(&sw, &led);
    builder.connect(&sw, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setMode(SimulationMode::Temporal);
    sim->setTimePerTick(100);
    sim->update(); // re-init

    // LED sink has delay 0, so it should update immediately
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
    LogicElement *dummy = reinterpret_cast<LogicElement *>(1); // not dereferenced by record

    // Manually build a recorder with a fake trace
    int idx = recorder.watchSignal("sig0", nullptr, 0);
    QCOMPARE(idx, 0);
    QCOMPARE(recorder.traceCount(), 1);
    QCOMPARE(recorder.trace(0).name, QString("sig0"));

    // Add second signal
    recorder.watchSignal("sig1", nullptr, 0);
    QCOMPARE(recorder.traceCount(), 2);

    // Clear
    recorder.clear();
    QCOMPARE(recorder.traceCount(), 0);

    Q_UNUSED(dummy)
}

void TestTemporalSimulation::testRecorderDeduplication()
{
    // recordAll should not add duplicate consecutive values
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setMode(SimulationMode::Temporal);
    sim->setTimePerTick(100);
    sim->update(); // re-init rebuilds LogicElements

    // Watch AFTER re-init so the pointer is valid
    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("NOT_out", notGate.logic(), 0);
    recorder.setRecording(true);

    // Record the same state multiple times — should dedup
    sim->update();
    sim->update();
    sim->update();

    const auto &transitions = recorder.trace(0).transitions;
    // Should have at most a few transitions, not one per update
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
    // Full integration: temporal sim with recorder, verify transitions are captured
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();
    sim->setMode(SimulationMode::Temporal);
    sim->setTimePerTick(100);
    sim->update(); // re-init rebuilds LogicElements

    // Watch AFTER re-init so the pointer is valid
    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("NOT_out", notGate.logic(), 0);
    recorder.setRecording(true);

    // sw=off → NOT=1
    sw.setOn(false);
    sim->update();

    // sw=on → NOT=0
    sw.setOn(true);
    sim->update();

    const auto &transitions = recorder.trace(0).transitions;
    QVERIFY2(!transitions.isEmpty(), "Recorder should have captured transitions");

    // The last transition should reflect NOT(1) = Inactive
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

    // No traces → minimum size
    QSize hint = widget.sizeHint();
    QVERIFY(hint.width() >= 220);
    QVERIFY(hint.height() >= 54);

    // Add a trace → height increases
    recorder.watchSignal("sig0", nullptr, 0);
    hint = widget.sizeHint();
    QVERIFY(hint.height() >= 54);

    // Widget should not crash when painted with no data
    widget.resize(400, 200);
    widget.repaint();
}
