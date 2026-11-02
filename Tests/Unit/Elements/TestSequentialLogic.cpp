// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestSequentialLogic.h"

#include <functional>
#include <memory>

#include <QDataStream>
#include <QImage>
#include <QPainter>
#include <QScopeGuard>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryDir>

#include "App/Core/ThemeManager.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Presentation/ElementPictures.h"
#include "App/Presentation/ElementPresenter.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "Tests/Common/TestUtils.h"

void TestSequentialLogic::testDFlipFlop()
{
    Element elm{ElementType::DFlipFlop};
    QCOMPARE(elm.inputSize(), 4);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 4);
    QCOMPARE(elm.maxInputSize(), 4);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::DFlipFlop);
}

void TestSequentialLogic::testDLatch()
{
    Element elm{ElementType::DLatch};
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.maxInputSize(), 2);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::DLatch);
}

void TestSequentialLogic::testJKFlipFlop()
{
    Element elm{ElementType::JKFlipFlop};
    QCOMPARE(elm.inputSize(), 5);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 5);
    QCOMPARE(elm.maxInputSize(), 5);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::JKFlipFlop);
}

void TestSequentialLogic::testSRFlipFlop()
{
    Element elm{ElementType::SRFlipFlop};
    QCOMPARE(elm.inputSize(), 5);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 5);
    QCOMPARE(elm.maxInputSize(), 5);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::SRFlipFlop);
}

void TestSequentialLogic::testSRLatch()
{
    Element elm{ElementType::SRLatch};
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.maxInputSize(), 2);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::SRLatch);
}

void TestSequentialLogic::testTFlipFlop()
{
    Element elm{ElementType::TFlipFlop};
    QCOMPARE(elm.inputSize(), 4);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 4);
    QCOMPARE(elm.maxInputSize(), 4);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::TFlipFlop);
}

// ============================================================================
// Behavioral Tests
// ============================================================================

void TestSequentialLogic::testDFlipFlopBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swD{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element dff{ElementType::DFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swD, &swClk, &dff, &ledQ, &ledNQ);
    builder.connect(&swD, 0, &dff, 0);    // D
    builder.connect(&swClk, 0, &dff, 1);  // Clock
    builder.connect(&dff, 0, &ledQ, 0);   // Q
    builder.connect(&dff, 1, &ledNQ, 0);  // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // D=1: capture into m_lastValue, then rising edge → Q=1
    swD.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // Hold: D→0, no clock → Q stays 1
    swD.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // Clock with D=0 → Q=0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
}

void TestSequentialLogic::testDLatchBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swD{ElementType::InputSwitch};
    Element swEnable{ElementType::InputSwitch};
    Element dlatch{ElementType::DLatch};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swD, &swEnable, &dlatch, &ledQ, &ledNQ);
    builder.connect(&swD, 0, &dlatch, 0);      // D
    builder.connect(&swEnable, 0, &dlatch, 1); // Enable
    builder.connect(&dlatch, 0, &ledQ, 0);     // Q
    builder.connect(&dlatch, 1, &ledNQ, 0);    // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // Enable=1, D=1: transparent → Q=1
    swEnable.setOn(true);
    swD.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // Enable=0, D→0: latched → Q stays 1
    swEnable.setOn(false);
    swD.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // Enable=1, D=0: transparent again → Q=0
    swEnable.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
}

void TestSequentialLogic::testJKFlipFlopBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swJ{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element swK{ElementType::InputSwitch};
    Element jkff{ElementType::JKFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swJ, &swClk, &swK, &jkff, &ledQ, &ledNQ);
    builder.connect(&swJ, 0, &jkff, 0);   // J
    builder.connect(&swClk, 0, &jkff, 1); // Clock
    builder.connect(&swK, 0, &jkff, 2);   // K
    builder.connect(&jkff, 0, &ledQ, 0);  // Q
    builder.connect(&jkff, 1, &ledNQ, 0); // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // J=1, K=0: Set → Q=1
    swJ.setOn(true);
    swK.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // J=0, K=1: Reset → Q=0
    swJ.setOn(false);
    swK.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // J=1, K=1: Toggle → Q=1
    swJ.setOn(true);
    swK.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // J=1, K=1: Toggle again → Q=0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
}

void TestSequentialLogic::testSRFlipFlopBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swS{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element swR{ElementType::InputSwitch};
    Element srff{ElementType::SRFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swS, &swClk, &swR, &srff, &ledQ, &ledNQ);
    builder.connect(&swS, 0, &srff, 0);   // S
    builder.connect(&swClk, 0, &srff, 1); // Clock
    builder.connect(&swR, 0, &srff, 2);   // R
    builder.connect(&srff, 0, &ledQ, 0);  // Q
    builder.connect(&srff, 1, &ledNQ, 0); // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // S=1, R=0: Set → Q=1
    swS.setOn(true);
    swR.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // S=0, R=0: Hold → Q stays 1
    swS.setOn(false);
    swR.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // S=0, R=1: Reset → Q=0
    swR.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
}

void TestSequentialLogic::testSRLatchBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swS{ElementType::InputSwitch};
    Element swR{ElementType::InputSwitch};
    Element srlatch{ElementType::SRLatch};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swS, &swR, &srlatch, &ledQ, &ledNQ);
    builder.connect(&swS, 0, &srlatch, 0); // Set
    builder.connect(&swR, 0, &srlatch, 1); // Reset
    builder.connect(&srlatch, 0, &ledQ, 0);  // Q
    builder.connect(&srlatch, 1, &ledNQ, 0); // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // S=1, R=0: Set → Q=1
    swS.setOn(true);
    swR.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // S=0, R=0: Hold → Q stays 1
    swS.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // S=0, R=1: Reset → Q=0
    swR.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // S=1, R=1: Invalid state → Q=0, ~Q=0
    swS.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

void TestSequentialLogic::testTFlipFlopBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swT{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element tff{ElementType::TFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swT, &swClk, &tff, &ledQ, &ledNQ);
    builder.connect(&swT, 0, &tff, 0);    // T
    builder.connect(&swClk, 0, &tff, 1);  // Clock
    builder.connect(&tff, 0, &ledQ, 0);   // Q
    builder.connect(&tff, 1, &ledNQ, 0);  // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // T=0: clock pulse → no toggle (hold)
    swT.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // T=1: clock pulse → toggle Q=0→1
    swT.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // T=1: clock pulse → toggle Q=1→0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
}

void TestSequentialLogic::testSRFlipFlopWithOneOfSAndROpen()
{
    // An open S or R reads Inactive: the flip-flop sets with R open, and presets and resets with S open
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swClk{ElementType::InputSwitch};
    Element swS{ElementType::InputSwitch};
    Element swR{ElementType::InputSwitch};
    Element swPreset{ElementType::InputSwitch};
    Element setOnly{ElementType::SRFlipFlop}, resetOnly{ElementType::SRFlipFlop};
    Element ledSet{ElementType::Led}, ledReset{ElementType::Led};

    builder.add(&swClk, &swS, &swR, &swPreset, &setOnly, &resetOnly, &ledSet, &ledReset);
    builder.connect(&swS, 0, &setOnly, 0);        // S; R open
    builder.connect(&swClk, 0, &setOnly, 1);
    builder.connect(&setOnly, 0, &ledSet, 0);
    builder.connect(&swClk, 0, &resetOnly, 1);    // S open
    builder.connect(&swR, 0, &resetOnly, 2);      // R
    builder.connect(&swPreset, 0, &resetOnly, 3); // ~Preset
    builder.connect(&resetOnly, 0, &ledReset, 0);

    swPreset.setOn(true);
    auto *sim = builder.initSimulation();
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledSet), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledReset), Status::Inactive);

    swS.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledSet), Status::Active);

    swPreset.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledReset), Status::Active);
    swPreset.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledReset), Status::Active);

    swR.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledReset), Status::Inactive);
}

void TestSequentialLogic::testTFlipFlopWithTOpen()
{
    // An open T reads Inactive: the flip-flop presets, holds on a clock edge and clears
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swClk{ElementType::InputSwitch};
    Element swPreset{ElementType::InputSwitch};
    Element swClear{ElementType::InputSwitch};
    Element tff{ElementType::TFlipFlop};
    Element ledQ{ElementType::Led};

    builder.add(&swClk, &swPreset, &swClear, &tff, &ledQ);
    builder.connect(&swClk, 0, &tff, 1);    // T open
    builder.connect(&swPreset, 0, &tff, 2); // ~Preset
    builder.connect(&swClear, 0, &tff, 3);  // ~Clear
    builder.connect(&tff, 0, &ledQ, 0);

    swPreset.setOn(true);
    swClear.setOn(true);
    auto *sim = builder.initSimulation();
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);

    swPreset.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    swPreset.setOn(true);
    sim->update();

    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    swClear.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
}

// ============================================================================
// resetSimState() Tests
// ============================================================================
// resetSimState() is called by WaveformSimulator before each fresh BeWavedDolphin sweep;
// no BeWavedDolphin test circuit happens to include a flip-flop, so these exercise it
// directly: drive Q away from its power-on default, reset, and confirm both the output
// AND the edge-detection state came back cleanly (a subsequent real clock edge still
// behaves correctly, proving the kernel's history (Element::kernelState()) wasn't left corrupt).
//
// Each test drives an input change (setOn(false) on the data-like input) right after
// resetSimState(), exactly as WaveformSimulator::sweep() always does immediately after
// its own reset loop (it goes straight into driving the first column's input values).
// resetSimState() bypasses setOutputValue()'s change-detection (Simulation::update()'s
// idle-tick skip only wakes on a clock/input-element change), so calling it with no
// following input change is not a scenario any real caller produces.

void TestSequentialLogic::testDFlipFlopResetSimState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swD{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element dff{ElementType::DFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swD, &swClk, &dff, &ledQ, &ledNQ);
    builder.connect(&swD, 0, &dff, 0);
    builder.connect(&swClk, 0, &dff, 1);
    builder.connect(&dff, 0, &ledQ, 0);
    builder.connect(&dff, 1, &ledNQ, 0);

    auto *sim = builder.initSimulation();
    sim->update();

    swD.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    dff.resetSimState();
    swD.setOn(false); // wakes the simulation's idle-tick skip, as a real sweep column would
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // A fresh rising edge with D=1 must still set Q=1 correctly after the reset.
    swD.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

void TestSequentialLogic::testJKFlipFlopResetSimState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swJ{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element swK{ElementType::InputSwitch};
    Element jkff{ElementType::JKFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swJ, &swClk, &swK, &jkff, &ledQ, &ledNQ);
    builder.connect(&swJ, 0, &jkff, 0);
    builder.connect(&swClk, 0, &jkff, 1);
    builder.connect(&swK, 0, &jkff, 2);
    builder.connect(&jkff, 0, &ledQ, 0);
    builder.connect(&jkff, 1, &ledNQ, 0);

    auto *sim = builder.initSimulation();
    sim->update();

    swJ.setOn(true);
    swK.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    jkff.resetSimState();
    swJ.setOn(false); // wakes the simulation's idle-tick skip, as a real sweep column would
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // A fresh rising edge with J=1,K=0 must still Set correctly after the reset.
    swJ.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

void TestSequentialLogic::testSRFlipFlopResetSimState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swS{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element swR{ElementType::InputSwitch};
    Element srff{ElementType::SRFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swS, &swClk, &swR, &srff, &ledQ, &ledNQ);
    builder.connect(&swS, 0, &srff, 0);
    builder.connect(&swClk, 0, &srff, 1);
    builder.connect(&swR, 0, &srff, 2);
    builder.connect(&srff, 0, &ledQ, 0);
    builder.connect(&srff, 1, &ledNQ, 0);

    auto *sim = builder.initSimulation();
    sim->update();

    swS.setOn(true);
    swR.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    srff.resetSimState();
    swS.setOn(false); // wakes the simulation's idle-tick skip, as a real sweep column would
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // A fresh rising edge with S=1,R=0 must still Set correctly after the reset.
    swS.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

void TestSequentialLogic::testTFlipFlopResetSimState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swT{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element tff{ElementType::TFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swT, &swClk, &tff, &ledQ, &ledNQ);
    builder.connect(&swT, 0, &tff, 0);
    builder.connect(&swClk, 0, &tff, 1);
    builder.connect(&tff, 0, &ledQ, 0);
    builder.connect(&tff, 1, &ledNQ, 0);

    auto *sim = builder.initSimulation();
    sim->update();

    swT.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    tff.resetSimState();
    swT.setOn(false); // wakes the simulation's idle-tick skip, as a real sweep column would
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // A fresh rising edge with T=1 must still toggle correctly after the reset.
    swT.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

// ============================================================================
// Kernel History Tests
// ============================================================================

void TestSequentialLogic::testDFlipFlopLatchesTheDBeforeTheEdge()
{
    // The edge latches the D its flip-flop sampled at the evaluation before, not the D of the edge's own evaluation:
    // the zero setup time. A D that rises with the clock is latched at the next edge.
    Element dff{ElementType::DFlipFlop};
    ElementArray sources{ElementType::InputVcc, 4}; // D, Clock, ~Preset, ~Clear
    TestUtils::initElm(dff);
    for (int port = 0; port < sources.size(); ++port) {
        TestUtils::initSrc(sources[port]);
        dff.connectPredecessor(port, &sources[port], 0);
    }
    sources[2].setOutputValue(Status::Active);
    sources[3].setOutputValue(Status::Active);

    sources[0].setOutputValue(Status::Inactive);
    sources[1].setOutputValue(Status::Inactive);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), Status::Inactive);

    sources[0].setOutputValue(Status::Active);
    sources[1].setOutputValue(Status::Active);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), Status::Inactive);
    QCOMPARE(dff.outputValue(1), Status::Active);

    sources[1].setOutputValue(Status::Inactive);
    dff.updateLogic();
    sources[1].setOutputValue(Status::Active);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), Status::Active);
    QCOMPARE(dff.outputValue(1), Status::Inactive);
}

void TestSequentialLogic::testFirstEvaluationWithTheClockHighIsAnEdge_data()
{
    QTest::addColumn<ElementType>("type");
    QTest::addColumn<Status>("q");

    // The power-on history holds a low clock and the data the old flip-flops started with: a D flip-flop's high D,
    // a T flip-flop's high T and a JK flip-flop's high J and K, so a clock high at the first evaluation latches a
    // high Q from those, whatever the data inputs are; an SR flip-flop's S and R start low and keep its Q
    QTest::newRow("D") << ElementType::DFlipFlop << Status::Active;
    QTest::newRow("T") << ElementType::TFlipFlop << Status::Active;
    QTest::newRow("JK") << ElementType::JKFlipFlop << Status::Active;
    QTest::newRow("SR") << ElementType::SRFlipFlop << Status::Inactive;
}

void TestSequentialLogic::testFirstEvaluationWithTheClockHighIsAnEdge()
{
    QFETCH(ElementType, type);
    QFETCH(Status, q);

    // The clock is the second input and the overrides, ~Preset and ~Clear, the last two; every other input is data
    Element flipFlop{type};
    TestUtils::initElm(flipFlop);
    ElementArray sources{ElementType::InputVcc, flipFlop.inputSize()};
    for (int port = 0; port < sources.size(); ++port) {
        TestUtils::initSrc(sources[port]);
        const bool high = (port == 1) || (port >= sources.size() - 2);
        sources[port].setOutputValue(high ? Status::Active : Status::Inactive);
        flipFlop.connectPredecessor(port, &sources[port], 0);
    }

    flipFlop.updateLogic();
    QCOMPARE(flipFlop.outputValue(0), q);
    QCOMPARE(flipFlop.outputValue(1), (q == Status::Active) ? Status::Inactive : Status::Active);
}

void TestSequentialLogic::testFlipFlopHistoryFollowsResetSaveAndRestore()
{
    Element dff{ElementType::DFlipFlop};
    ElementArray sources{ElementType::InputVcc, 4}; // D, Clock, ~Preset, ~Clear
    TestUtils::initElm(dff);
    for (int port = 0; port < sources.size(); ++port) {
        TestUtils::initSrc(sources[port]);
        dff.connectPredecessor(port, &sources[port], 0);
    }
    sources[2].setOutputValue(Status::Active);
    sources[3].setOutputValue(Status::Active);

    sources[0].setOutputValue(Status::Active);
    sources[1].setOutputValue(Status::Inactive);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), Status::Inactive);

    // A save before an edge, restored after it, takes back the history with the outputs, so the edge comes again
    QVector<Status> saved;
    dff.saveSimState(saved);
    sources[1].setOutputValue(Status::Active);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), Status::Active);

    int cursor = 0;
    dff.restoreSimState(saved, cursor);
    QCOMPARE(cursor, saved.size());
    QCOMPARE(dff.outputValue(0), Status::Inactive);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), Status::Active);

    // A reset takes the history back to power-on's, whose low clock makes the clock still high an edge again, and
    // whose high D it latches although D is now low
    sources[0].setOutputValue(Status::Inactive);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), Status::Active);
    dff.resetSimState();
    QCOMPARE(dff.outputValue(0), Status::Inactive);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), Status::Active);
}

// ============================================================================
// Theme Tests
// ============================================================================

namespace TestSequentialLogicHelpers {

/// What \a element draws, unselected: its picture, as paint() gives it.
QImage drawn(Element &element)
{
    const bool selected = element.isSelected();
    element.setSelected(false);
    QImage image(96, 96, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    const QStyleOptionGraphicsItem option;
    element.paint(&painter, &option, nullptr);
    painter.end();
    element.setSelected(selected);
    return image;
}

/// What \a element's presenter draws of its body in \a theme, whichever theme is current.
QImage drawnIn(const Element &element, const ThemeAttributes &theme)
{
    QImage image(96, 96, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    ElementPresenter::of(element.elementType()).paintBody(painter, element, BodyContext{theme});
    painter.end();
    return image;
}

/// Makes each latch and flip-flop, whose picture is the theme's.
QList<std::function<Element *()>> themedTypes()
{
    return {
        [] { return ElementFactory::buildElement(ElementType::DFlipFlop); },
        [] { return ElementFactory::buildElement(ElementType::JKFlipFlop); },
        [] { return ElementFactory::buildElement(ElementType::SRFlipFlop); },
        [] { return ElementFactory::buildElement(ElementType::TFlipFlop); },
        [] { return ElementFactory::buildElement(ElementType::DLatch); },
        [] { return ElementFactory::buildElement(ElementType::SRLatch); },
    };
}

/// What a new element made by \a make draws in \a theme.
QImage drawnNew(const std::function<Element *()> &make, const Theme theme)
{
    ThemeManager::setTheme(theme);
    std::unique_ptr<Element> element(make());
    return drawn(*element);
}

} // namespace TestSequentialLogicHelpers

using TestSequentialLogicHelpers::drawn;
using TestSequentialLogicHelpers::drawnIn;
using TestSequentialLogicHelpers::drawnNew;
using TestSequentialLogicHelpers::themedTypes;

void TestSequentialLogic::testAThemedBodyIsDrawnInTheThemeHanded()
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });
    ThemeManager::setTheme(Theme::Dark);
    const ThemeAttributes dark = ThemeManager::attributes();
    ThemeManager::setTheme(Theme::Light);
    const ThemeAttributes light = ThemeManager::attributes();

    for (const auto &make : themedTypes()) {
        std::unique_ptr<Element> element(make());
        const QString name = element->translatedName();
        const QRectF body = element->body().body;

        // Each theme's body drawn while the other is current is the one drawn while it is
        ThemeManager::setTheme(Theme::Light);
        const QImage darkWhileLight = drawnIn(*element, dark);
        const QImage lightWhileLight = drawnIn(*element, light);
        ThemeManager::setTheme(Theme::Dark);
        element->updateTheme();
        QVERIFY2(drawnIn(*element, dark) == darkWhileLight, qPrintable(name));
        QVERIFY2(drawnIn(*element, light) == lightWhileLight, qPrintable(name));
        QVERIFY2(darkWhileLight != lightWhileLight, qPrintable(name));

        // The body is measured the same in either theme, on the picture the element names
        QVERIFY2(element->body().body == body, qPrintable(name));
        const PictureVariant *lightPicture = ElementPictures::instance().shown(*element, Theme::Light);
        const PictureVariant *darkPicture = ElementPictures::instance().shown(*element, Theme::Dark);
        QVERIFY2(lightPicture && darkPicture && (lightPicture != darkPicture), qPrintable(name));
        QCOMPARE(darkPicture->pixmap.size(), lightPicture->pixmap.size());
        QCOMPARE(QRectF(lightPicture->pixmap.rect()), body);
    }
}

void TestSequentialLogic::testThemedPictureFollowsTheThemeThroughARefresh()
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });

    for (const auto &make : themedTypes()) {
        const QImage dark = drawnNew(make, Theme::Dark);
        ThemeManager::setTheme(Theme::Light);
        std::unique_ptr<Element> element(make());
        const QString name = element->translatedName();
        QVERIFY2(drawn(*element) != dark, qPrintable(name));

        ThemeManager::setTheme(Theme::Dark);
        element->updateTheme();
        QVERIFY2(drawn(*element) == dark, qPrintable(name));
        element->refresh();
        QVERIFY2(drawn(*element) == dark, qPrintable(name));
    }
}

void TestSequentialLogic::testThemeSwitchKeepsACustomAppearance()
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString png = dir.filePath("custom.png");
    QImage red(64, 64, QImage::Format_ARGB32);
    red.fill(Qt::red);
    QVERIFY(red.save(png));

    for (const auto &make : themedTypes()) {
        const QImage dark = drawnNew(make, Theme::Dark);
        ThemeManager::setTheme(Theme::Light);
        std::unique_ptr<Element> element(make());
        const QString name = element->translatedName();
        element->setAppearance(false, png);
        const QImage custom = drawn(*element);

        ThemeManager::setTheme(Theme::Dark);
        element->updateTheme();
        QVERIFY2(drawn(*element) == custom, qPrintable(name));
        element->refresh();
        QVERIFY2(drawn(*element) == custom, qPrintable(name));
        QCOMPARE(element->externalFiles(), QStringList{png});

        // Back to the built-in appearance, it is the dark one
        element->setAppearance(true, {});
        QVERIFY2(drawn(*element) == dark, qPrintable(name));
    }
}

void TestSequentialLogic::testUndoAfterAThemeSwitchShowsTheCurrentTheme()
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });

    QList<QImage> dark;
    for (const auto &make : themedTypes()) {
        dark.append(drawnNew(make, Theme::Dark));
    }

    // A label edit of each, recorded while the theme is light
    ThemeManager::setTheme(Theme::Light);
    Scene scene;
    QList<Element *> elements;
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (const auto &make : themedTypes()) {
        auto *element = make();
        element->setPos(0, 128 * static_cast<int>(elements.size()));
        scene.addItem(element);
        element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
        element->setLabel("Q");
        elements.append(element);
    }
    scene.receiveCommand(new UpdateCommand(elements, oldData, &scene));

    ThemeManager::setTheme(Theme::Dark);
    scene.updateTheme();
    for (int i = 0; i < elements.size(); ++i) {
        QVERIFY2(drawn(*elements.at(i)) == dark.at(i), qPrintable(elements.at(i)->translatedName()));
    }

    scene.undoStack()->undo();
    for (int i = 0; i < elements.size(); ++i) {
        auto *element = elements.at(i);
        QCOMPARE(element->label(), QString());
        QVERIFY2(drawn(*element) == dark.at(i), qPrintable(element->translatedName()));
        QVERIFY2(element->externalFiles().isEmpty(), qPrintable(element->translatedName()));
    }

    scene.undoStack()->redo();
    for (int i = 0; i < elements.size(); ++i) {
        auto *element = elements.at(i);
        QCOMPARE(element->label(), QString("Q"));
        QVERIFY2(drawn(*element) == dark.at(i), qPrintable(element->translatedName()));
    }
}
