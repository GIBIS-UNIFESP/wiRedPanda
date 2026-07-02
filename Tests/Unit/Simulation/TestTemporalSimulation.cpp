// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestTemporalSimulation.h"

#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QVersionNumber>

#include "App/BeWavedDolphin/LiveAnalyzer.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/SimEvent.h"
#include "App/Simulation/SimTime.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/WaveformRecorder.h"
#include "Tests/Common/TestUtils.h"

using namespace TestUtils;

namespace {

/// Loads a top-level example circuit from Examples/ into \a ws. Returns false if the file is
/// missing/unreadable (callers QSKIP), matching how the other fixture-loading tests guard.
bool loadExample(WorkSpace &ws, const QString &fileName)
{
    const QString path = TestUtils::examplesDir() + fileName;
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QDataStream stream(&file);
    const QVersionNumber version = Serialization::readPandaHeader(stream);
    ws.load(stream, version, QFileInfo(path).absolutePath());
    return true;
}

/// Returns the first scene element labelled \a label, or nullptr.
GraphicElement *byLabel(WorkSpace &ws, const QString &label)
{
    for (auto *elm : ws.scene()->elements()) {
        if (elm && elm->label() == label) {
            return elm;
        }
    }
    return nullptr;
}

} // namespace

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

void TestTemporalSimulation::testNestedICDelayAccumulation()
{
    // The 4-bit ripple adder is a genuinely NESTED IC: it instantiates level2_full_adder_1bit
    // sub-ICs, each itself built from gates. Loading it flattens recursively (IC → IC → gates),
    // so propagation delays must accumulate down the carry chain across TWO IC boundary levels.
    // We drive a worst-case ripple (1111 + 0001 = 10000): CarryOut depends on the carry rippling
    // through all four nested full-adders, the deepest path — its delay must be far longer than a
    // single gate, proving delays survive nested flattening rather than collapsing to a black box.
    const QString icPath = cpuComponentsDir() + QStringLiteral("level4_ripple_adder_4bit.panda");
    if (!QFile::exists(icPath)) {
        QSKIP("level4_ripple_adder_4bit.panda fixture not found");
    }

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swA[4], swB[4], carryIn;
    Led ledCarryOut;
    builder.add(&swA[0], &swA[1], &swA[2], &swA[3]);
    builder.add(&swB[0], &swB[1], &swB[2], &swB[3]);
    builder.add(&carryIn, &ledCarryOut);

    auto *ic = new IC();
    ic->loadFile(icPath, QFileInfo(icPath).absolutePath());
    builder.add(ic);

    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, ic, QStringLiteral("A[%1]").arg(i));
        builder.connect(&swB[i], 0, ic, QStringLiteral("B[%1]").arg(i));
    }
    builder.connect(&carryIn, 0, ic, QStringLiteral("CarryIn"));
    builder.connect(ic, QStringLiteral("CarryOut"), &ledCarryOut, 0);

    auto *sim = builder.initSimulation();

    const auto setOperands = [&](bool aHigh, bool bLow) {
        for (int i = 0; i < 4; ++i) {
            swA[i].setOn(aHigh);                 // A = 1111 when aHigh
            swB[i].setOn(bLow && i == 0);        // B = 0001 when bLow, else 0000
        }
        carryIn.setOn(false);
    };

    // A wide tick settles the whole nested netlist: 1111 + 0001 = 10000 ⇒ CarryOut = 1.
    sim->setTimePerTick(500);
    setOperands(false, false); // 0000 + 0000 = 0 ⇒ CarryOut 0
    sim->update();
    QCOMPARE(inputStatus(&ledCarryOut), false);

    setOperands(true, true);   // 1111 + 0001 ⇒ CarryOut 1
    sim->update();
    QCOMPARE(inputStatus(&ledCarryOut), true);

    // Now prove the carry-chain delay actually accumulates through the nested boundaries: with
    // 1 ns ticks, CarryOut must NOT flip within a few ns of the input change (the ripple has to
    // cross four nested full-adders), and must settle only well after that.
    sim->setTimePerTick(1);
    setOperands(false, false);
    for (int i = 0; i < 600; ++i) { sim->update(); } // settle CarryOut back to 0
    QCOMPARE(inputStatus(&ledCarryOut), false);

    setOperands(true, true);
    for (int i = 0; i < 4; ++i) { sim->update(); }   // 4 ns ≪ the rippled carry-chain delay
    QCOMPARE(inputStatus(&ledCarryOut), false);   // nested delays not yet elapsed

    for (int i = 0; i < 600; ++i) { sim->update(); } // well past the full ripple
    QCOMPARE(inputStatus(&ledCarryOut), true);    // carry rippled through nested ICs
}

void TestTemporalSimulation::testNestedICDelayAccumulationDeepNesting()
{
    // testNestedICDelayAccumulation only exercises 2 levels of IC nesting (ripple-adder IC ->
    // full-adder IC -> gates). level6_alu_8bit nests 4 levels deep: level6_alu_8bit ->
    // level4_ripple_alu_4bit -> level4_ripple_adder_4bit -> level2_full_adder_1bit -> gates.
    // Drive a worst-case ripple-carry ADD (0xFF + 0x01, opcode 0) and prove the Carry output's
    // delay survives flattening through all 4 boundary levels, not just 2.
    const QString icPath = cpuComponentsDir() + QStringLiteral("level6_alu_8bit.panda");
    if (!QFile::exists(icPath)) {
        QSKIP("level6_alu_8bit.panda fixture not found");
    }

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    QVector<InputSwitch *> aInputs, bInputs, opcodeInputs;
    for (int i = 0; i < 8; ++i) {
        auto *a = new InputSwitch();
        builder.add(a);
        aInputs.append(a);
        auto *b = new InputSwitch();
        builder.add(b);
        bInputs.append(b);
    }
    for (int i = 0; i < 3; ++i) {
        auto *op = new InputSwitch();
        builder.add(op);
        opcodeInputs.append(op);
    }
    Led carryFlag;
    builder.add(&carryFlag);

    auto *ic = new IC();
    ic->loadFile(icPath, QFileInfo(icPath).absolutePath());
    builder.add(ic);

    for (int i = 0; i < 8; ++i) {
        builder.connect(aInputs[i], 0, ic, QStringLiteral("A[%1]").arg(i));
        builder.connect(bInputs[i], 0, ic, QStringLiteral("B[%1]").arg(i));
    }
    for (int i = 0; i < 3; ++i) {
        builder.connect(opcodeInputs[i], 0, ic, QStringLiteral("OpCode[%1]").arg(i));
    }
    builder.connect(ic, QStringLiteral("Carry"), &carryFlag, 0);

    auto *sim = builder.initSimulation();

    const auto setOperands = [&](bool aHigh, bool bLow) {
        for (int i = 0; i < 8; ++i) {
            aInputs[i]->setOn(aHigh);         // A = 0xFF when aHigh
            bInputs[i]->setOn(bLow && i == 0); // B = 0x01 when bLow, else 0x00
        }
        for (auto *op : opcodeInputs) {
            op->setOn(false); // OpCode = 0 (ADD)
        }
    };

    // A wide tick settles the whole 4-level-nested netlist: 0xFF + 0x01 = 0x00, Carry = 1.
    sim->setTimePerTick(500);
    setOperands(false, false); // 0x00 + 0x00 = 0x00 ⇒ Carry 0
    sim->update();
    QCOMPARE(inputStatus(&carryFlag), false);

    setOperands(true, true); // 0xFF + 0x01 ⇒ Carry 1
    sim->update();
    QCOMPARE(inputStatus(&carryFlag), true);

    // Now prove the delay actually accumulates through all 4 nesting levels: with 1 ns ticks,
    // Carry must NOT flip within a few ns of the input change, and must settle only well after.
    sim->setTimePerTick(1);
    setOperands(false, false);
    for (int i = 0; i < 1200; ++i) { sim->update(); } // settle Carry back to 0
    QCOMPARE(inputStatus(&carryFlag), false);

    setOperands(true, true);
    for (int i = 0; i < 4; ++i) { sim->update(); }    // 4 ns ≪ the 4-level rippled carry delay
    QCOMPARE(inputStatus(&carryFlag), false);      // nested delays not yet elapsed

    for (int i = 0; i < 1200; ++i) { sim->update(); } // well past the full 4-level ripple
    QCOMPARE(inputStatus(&carryFlag), true);       // carry rippled through all 4 nested ICs
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

void TestTemporalSimulation::testRecorderSubTickResolution()
{
    // recordAll() runs once per DISTINCT timestamp during the drain, not once per tick — so even a
    // single wide tick captures every ns-resolution transition inside it. A 3-inverter ring with
    // per-gate delays is a clean source of many transitions per tick: one 100 ns update must record
    // several transitions at distinct timestamps within that one tick (unlike testRecorderIntegration,
    // whose 100 ns ticks each hold a single change).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Not n1, n2, n3;
    builder.add(&n1, &n2, &n3);
    builder.connect(&n1, 0, &n2, 0);
    builder.connect(&n2, 0, &n3, 0);
    builder.connect(&n3, 0, &n1, 0); // ring oscillator

    auto *sim = builder.initSimulation();
    sim->setTimePerTick(100);
    sim->setElementDelay(&n1, 5);
    sim->setElementDelay(&n2, 5);
    sim->setElementDelay(&n3, 5);

    sim->update(); // init + first oscillating window (not yet recorded)

    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("n1", &n1, 0);
    recorder.setRecording(true);

    const SimTime tickStart = sim->currentTime();
    sim->update(); // exactly ONE 100 ns tick, fully recorded
    const SimTime tickEnd = sim->currentTime();

    const auto &transitions = recorder.trace(0).transitions;

    // Collect the distinct timestamps captured during this single tick.
    QVector<SimTime> stamps;
    for (const auto &tr : transitions) {
        if (tr.first > tickStart && tr.first <= tickEnd && (stamps.isEmpty() || stamps.last() != tr.first)) {
            stamps.append(tr.first);
        }
    }

    QVERIFY2(stamps.size() >= 3, qPrintable(QString(
        "Expected ≥3 distinct-timestamp transitions within one %1 ns tick, got %2")
        .arg(static_cast<long long>(tickEnd - tickStart)).arg(stamps.size())));

    // Those timestamps must be strictly increasing and all inside the single tick window —
    // i.e. genuine sub-tick (ns) resolution, not one transition pinned per tick boundary.
    for (int i = 1; i < stamps.size(); ++i) {
        QVERIFY(stamps[i] > stamps[i - 1]);
    }
    QVERIFY(stamps.last() - stamps.first() < (tickEnd - tickStart));
}

void TestTemporalSimulation::testRecorderStatusAtEmptyTrace()
{
    // statusAt() on a trace with no recorded transitions returns Inactive (the documented
    // baseline), not a crash — the empty-trace boundary of the binary search.
    SignalTrace trace;
    trace.name = "empty";
    QVERIFY(trace.transitions.isEmpty());
    QCOMPARE(trace.statusAt(0), Status::Inactive);
    QCOMPARE(trace.statusAt(100), Status::Inactive);
}

void TestTemporalSimulation::testRecorderRecordAllSkipsNullAndCollapsesSameTime()
{
    // Two recordAll() edge paths:
    //  (a) a watched signal with a null element is skipped (no transition recorded);
    //  (b) recording the SAME timestamp twice with different values collapses into one
    //      transition (a zero-delay element firing on a tick boundary must not emit a
    //      zero-width glitch).
    WaveformRecorder recorder;

    // (a) null-logic trace — recordAll must skip it without recording or crashing.
    recorder.watchSignal("null", nullptr, 0);
    recorder.setRecording(true);
    recorder.recordAll(10);
    QCOMPARE(recorder.trace(0).transitions.size(), qsizetype(0));

    // (b) same-timestamp collapse on a real element whose output we drive directly. The element
    // must be sim-initialized (so its output vector exists) before setOutputValue takes effect.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *elm = new Not();
    builder.add(sw, elm);
    builder.connect(sw, 0, elm, 0);
    builder.initSimulation();

    const int idx = recorder.watchSignal("n", elm, 0);
    elm->setOutputValue(0, Status::Active);
    recorder.recordAll(20);              // appends (20, Active)
    elm->setOutputValue(0, Status::Inactive);
    recorder.recordAll(20);             // same time, new value → collapse, not a second entry

    const auto &transitions = recorder.trace(idx).transitions;
    int countAt20 = 0;
    Status valueAt20 = Status::Active;
    for (const auto &tr : transitions) {
        if (tr.first == 20) {
            ++countAt20;
            valueAt20 = tr.second;
        }
    }
    QCOMPARE(countAt20, 1);                 // collapsed into a single transition at t=20
    QCOMPARE(valueAt20, Status::Inactive);  // holding the latest value
}

// ============================================================================
// Live Analyzer canvas
// ============================================================================

void TestTemporalSimulation::testWidgetSizeHint()
{
    WaveformRecorder recorder;
    AnalyzerCanvas widget;
    widget.setRecorder(&recorder);

    // No traces → at least the minimum size (one trace row; the ruler is a separate widget).
    QSize hint = widget.sizeHint();
    QVERIFY(hint.width() >= 100);
    QVERIFY(hint.height() >= AnalyzerLayout::TraceHeight);

    // Adding a trace keeps a valid size.
    recorder.watchSignal("sig0", nullptr, 0);
    hint = widget.sizeHint();
    QVERIFY(hint.height() >= AnalyzerLayout::TraceHeight);

    // Painting with no recorded data must not crash.
    widget.resize(400, 200);
    widget.repaint();
}

void TestTemporalSimulation::testWaveformWidgetZoom()
{
    // The Live Analyzer's zoom controls (the +/- buttons) drive the canvas zoom level.
    // Verify the widget-level behaviour: zoomIn doubles, zoomOut halves the pixels-per-ns scale.
    WaveformRecorder recorder;
    AnalyzerCanvas widget;
    widget.setRecorder(&recorder);

    widget.setPixelsPerNs(4.0);
    QCOMPARE(widget.pixelsPerNs(), 4.0);

    widget.zoomIn();  // ×2
    QCOMPARE(widget.pixelsPerNs(), 8.0);

    widget.zoomOut(); // ÷2
    QCOMPARE(widget.pixelsPerNs(), 4.0);

    widget.zoomOut(); // ÷2 again
    QCOMPARE(widget.pixelsPerNs(), 2.0);
}

void TestTemporalSimulation::testWidgetCanvasClampedAtQtLimit()
{
    // Any attempt to set a minimum size beyond QWIDGETSIZE_MAX makes Qt emit this warning —
    // with the pre-fix code that happened on EVERY repaint of a long recording. Fail the
    // test if a single one appears.
    QTest::failOnWarning(QRegularExpression(".*largest allowed size.*"));

    // A recording whose timeline (6 s of sim time) at maximum zoom (10 px/ns) demands a
    // 6e10 px canvas — far beyond QWIDGETSIZE_MAX (16'777'215).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *elm = new Not();
    builder.add(sw, elm);
    builder.connect(sw, 0, elm, 0);
    builder.initSimulation();

    WaveformRecorder recorder;
    recorder.watchSignal("n", elm, 0);
    recorder.setRecording(true);
    elm->setOutputValue(0, Status::Active);
    recorder.recordAll(0);
    elm->setOutputValue(0, Status::Inactive);
    recorder.recordAll(6'000'000'000);

    AnalyzerCanvas widget;
    widget.setRecorder(&recorder);
    widget.setPixelsPerNs(10.0);

    // The size hint clamps at the Qt limit instead of demanding the impossible width.
    const QSize hint = widget.sizeHint();
    QCOMPARE(hint.width(), QWIDGETSIZE_MAX);

    // The canvas window slides to keep the NEWEST data on-canvas at the chosen zoom.
    const SimTime origin = widget.timeOrigin();
    QVERIFY(origin > 0);
    QVERIFY(origin < recorder.maxTime());
    QVERIFY(static_cast<double>(recorder.maxTime() - origin) * widget.pixelsPerNs()
            <= static_cast<double>(QWIDGETSIZE_MAX));

    // Painting an exposed strip must neither warn nor render the whole 16.7M px canvas.
    // grab() with an explicit rect drives paintEvent() with that exposed region.
    const QPixmap strip = widget.grab(QRect(0, 0, 400, 100));
    QVERIFY(!strip.isNull());

    // A short recording keeps the classic fixed mapping (origin pinned at t = 0).
    recorder.resetTimeline();
    elm->setOutputValue(0, Status::Active);
    recorder.recordAll(100);
    QCOMPARE(widget.timeOrigin(), SimTime(0));
}

void TestTemporalSimulation::testCanvasWheelZoomRequiresCtrl()
{
    // With vertical scrolling in the analyzer, a plain wheel must SCROLL (the canvas
    // ignores it so the surrounding scroll area handles it) — only Ctrl+wheel zooms,
    // the standard waveform-tool convention. The canvas does not rescale itself: it
    // emits a zoomStepRequested carrying the cursor position, and the hosting panel
    // applies the zoom anchored on the sim-time under the cursor.
    WaveformRecorder recorder;
    AnalyzerCanvas canvas;
    canvas.setRecorder(&recorder);
    canvas.setPixelsPerNs(1.0);
    QSignalSpy requests(&canvas, &AnalyzerCanvas::zoomStepRequested);

    auto sendWheel = [&canvas](Qt::KeyboardModifiers modifiers, int deltaY) {
        QWheelEvent event(QPointF(10, 10), QPointF(10, 10), QPoint(), QPoint(0, deltaY),
                          Qt::NoButton, modifiers, Qt::NoScrollPhase, false);
        QCoreApplication::sendEvent(&canvas, &event);
        return event.isAccepted();
    };

    const bool plainAccepted = sendWheel(Qt::NoModifier, 120);
    QVERIFY2(!plainAccepted, "plain wheel must be ignored so the scroll area scrolls");
    QCOMPARE(requests.count(), 0);
    QCOMPARE(canvas.pixelsPerNs(), 1.0);

    const bool ctrlAccepted = sendWheel(Qt::ControlModifier, 120);
    QVERIFY2(ctrlAccepted, "Ctrl+wheel must be consumed by the zoom request");
    QCOMPARE(requests.count(), 1);
    QCOMPARE(requests.last().at(0).toInt(), 1);      // wheel up → zoom in
    QCOMPARE(requests.last().at(1).toDouble(), 10.0); // cursor's canvas x
    QCOMPARE(canvas.pixelsPerNs(), 1.0);              // the panel rescales, not the canvas

    sendWheel(Qt::ControlModifier, -120);
    QCOMPARE(requests.count(), 2);
    QCOMPARE(requests.last().at(0).toInt(), -1);      // wheel down → zoom out
}

void TestTemporalSimulation::testCanvasPreRecordRegionBlank()
{
    // Watching typically starts mid-run: nothing is known before a trace's first recorded
    // sample, so that region must stay blank. The pre-fix renderer drew a definite LOW
    // there (statusAt() returns Inactive before the first sample), which made any view of
    // the pre-record region — e.g. the unanchored-zoom drift to t = 0 — read as "every
    // signal is low" instead of "no data here".
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *sw = new InputSwitch();
    auto *elm = new Not();
    builder.add(sw, elm);
    builder.connect(sw, 0, elm, 0);
    builder.initSimulation();

    WaveformRecorder recorder;
    recorder.watchSignal("n", elm, 0);
    recorder.setRecording(true);
    elm->setOutputValue(0, Status::Active);
    recorder.recordAll(600); // first sample: HIGH at t = 600 ns
    elm->setOutputValue(0, Status::Inactive);
    recorder.recordAll(800);

    AnalyzerCanvas canvas;
    canvas.setRecorder(&recorder);
    canvas.setPixelsPerNs(1.0);     // x == t in pixels
    canvas.grab(QRect(0, 0, 1, 1)); // settle the canvas to its hint (maxTime + pad wide)
    const QImage img = canvas.grab().toImage();
    QVERIFY(img.width() >= 800);

    const QRgb traceRgb = AnalyzerLayout::traceColor(0).rgb();
    auto columnRangeHasTraceColor = [&](int x0, int x1) {
        for (int x = x0; x <= x1 && x < img.width(); ++x) {
            for (int y = 0; y < img.height(); ++y) {
                if (img.pixel(x, y) == traceRgb) {
                    return true;
                }
            }
        }
        return false;
    };

    // ±20 px margins around the first sample absorb the 2 px pen and rounding.
    QVERIFY2(!columnRangeHasTraceColor(0, 580),
             "the region before the first recorded sample must stay blank");
    QVERIFY2(columnRangeHasTraceColor(620, 780),
             "the recorded region must draw the trace");
}

void TestTemporalSimulation::testAnalyzerTraceColorPalette()
{
    // Per-channel colors like a real logic analyzer: the first 8 channels are pairwise
    // distinct (adjacent traces distinguishable) and the palette cycles at 8.
    for (int i = 0; i < 8; ++i) {
        for (int j = i + 1; j < 8; ++j) {
            QVERIFY2(AnalyzerLayout::traceColor(i) != AnalyzerLayout::traceColor(j),
                     qPrintable(QString("channels %1 and %2 share a color").arg(i).arg(j)));
        }
    }
    QCOMPARE(AnalyzerLayout::traceColor(8), AnalyzerLayout::traceColor(0));
    QCOMPARE(AnalyzerLayout::traceColor(15), AnalyzerLayout::traceColor(7));
}

// ============================================================================
// Temporal example circuits (Examples/temporal_*.panda)
// ============================================================================

void TestTemporalSimulation::testExampleRingOscillator()
{
    // The shipped ring-oscillator example: three inverters in a loop. In functional mode the
    // zero-delay loop has no stable value and canonicalizes to Unknown; in temporal mode the
    // per-gate delays spread the toggles over time, so it oscillates cleanly.
    WorkSpace ws;
    if (!loadExample(ws, "temporal_ring_oscillator.panda")) {
        QSKIP("temporal_ring_oscillator.panda not found");
    }
    auto *sim = ws.scene()->simulation();
    auto *n1 = byLabel(ws, "n1");
    QVERIFY2(n1 && n1->elementType() == ElementType::Not, "ring inverter n1 missing");

    // Functional mode: the loop collapses to Unknown.
    sim->initialize();
    sim->update();
    QCOMPARE(n1->outputValue(0), Status::Unknown);

    // Temporal mode, settled fresh (initialize resets outputs to Inactive so it can start):
    // watch n1 and let it run; it must oscillate (>=3 transitions) and never go Unknown.
    sim->setTimePerTick(1);
    sim->initialize();
    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("n1", n1, 0);
    recorder.setRecording(true);
    for (int i = 0; i < 150; ++i) {
        sim->update();
    }

    const auto &transitions = recorder.trace(0).transitions;
    bool sawUnknown = false;
    for (const auto &tr : transitions) {
        if (tr.second == Status::Unknown) {
            sawUnknown = true;
        }
    }
    QVERIFY2(!sawUnknown, "temporal ring went Unknown (hit the oscillation cap)");
    QVERIFY2(transitions.size() >= 4, qPrintable(QString(
        "ring did not oscillate: only %1 recorded values").arg(transitions.size())));
}

void TestTemporalSimulation::testExampleStaticHazard()
{
    // The shipped static-hazard example: F = A OR (NOT A), a tautology, but the inverted path is
    // slower (tuned via per-element propagation delays) so a falling A produces a transient glitch
    // on F that only appears in temporal mode.
    WorkSpace ws;
    if (!loadExample(ws, "temporal_static_hazard.panda")) {
        QSKIP("temporal_static_hazard.panda not found");
    }
    auto *sim = ws.scene()->simulation();
    auto *swA = byLabel(ws, "A");
    auto *orGate = byLabel(ws, "reconv");
    auto *notGate = byLabel(ws, "inv");
    QVERIFY2(swA && orGate && notGate, "hazard elements missing");

    // The tuned delays must have round-tripped through the .panda.
    QCOMPARE(notGate->propagationDelay(), SimTime(10));
    QCOMPARE(orGate->propagationDelay(), SimTime(3));

    sim->setTimePerTick(100); // one wide tick drains the whole transient
    sim->initialize();
    static_cast<InputSwitch *>(swA)->setOn(true);
    sim->update(); // settle F = 1
    QCOMPARE(orGate->outputValue(0), Status::Active);

    auto &recorder = sim->waveformRecorder();
    recorder.watchSignal("F", orGate, 0);
    recorder.setRecording(true);

    static_cast<InputSwitch *>(swA)->setOn(false); // hazardous falling edge
    sim->update();

    bool sawLow = false;
    for (const auto &tr : recorder.trace(0).transitions) {
        if (tr.second == Status::Inactive) {
            sawLow = true;
        }
    }
    QVERIFY2(sawLow, "static-1 hazard glitch was not produced");
    QCOMPARE(orGate->outputValue(0), Status::Active); // tautology: settles back to 1
}

void TestTemporalSimulation::testExampleGateDelayChain()
{
    // The shipped gate-delay-chain example: A -> n1 -> n2 -> n3 -> n4. An edge on A reaches n4
    // only after the cumulative delay of all four inverters, strictly later than it reaches n1.
    WorkSpace ws;
    if (!loadExample(ws, "temporal_gate_delay_chain.panda")) {
        QSKIP("temporal_gate_delay_chain.panda not found");
    }
    auto *sim = ws.scene()->simulation();
    auto *swA = byLabel(ws, "A");
    auto *n1 = byLabel(ws, "n1");
    auto *n4 = byLabel(ws, "n4");
    QVERIFY2(swA && n1 && n4, "chain elements missing");

    sim->setTimePerTick(1);
    sim->initialize();
    static_cast<InputSwitch *>(swA)->setOn(false);
    for (int i = 0; i < 30; ++i) {
        sim->update(); // settle: n1 = high, n4 = low
    }
    const Status n1Base = n1->outputValue(0);
    const Status n4Base = n4->outputValue(0);

    // Rising edge on A; poll each tap so we record the sim-time at which it first changes.
    static_cast<InputSwitch *>(swA)->setOn(true);
    SimTime tN1 = SIM_TIME_UNSET;
    SimTime tN4 = SIM_TIME_UNSET;
    for (int i = 0; i < 60; ++i) {
        sim->update();
        if (tN1 == SIM_TIME_UNSET && n1->outputValue(0) != n1Base) {
            tN1 = sim->currentTime();
        }
        if (tN4 == SIM_TIME_UNSET && n4->outputValue(0) != n4Base) {
            tN4 = sim->currentTime();
        }
    }

    QVERIFY2(tN1 != SIM_TIME_UNSET && tN4 != SIM_TIME_UNSET, "chain taps never changed");
    QVERIFY2(tN4 > tN1, qPrintable(QString(
        "n4 (%1 ns) did not lag n1 (%2 ns): cumulative delay not observed").arg(tN4).arg(tN1)));
}
